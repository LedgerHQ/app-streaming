#!/usr/bin/env python3

import argparse
import logging
import zipfile

from construct import Bytes, Int8ul, Int16ul, Int32ul, Struct
from typing import Any, Dict

from comm import Apdu, exchange, get_client, import_ledgerwallet
from app import App, device_sign_app
from hsm import hsm_sign_app
from manifest import Manifest
from merkletree import Entry, MerkleTree
from server import Server


class Page:
    def __init__(self, data: bytes, mac: bytes, iv=0, read_only=False) -> None:
        self.read_only = False
        self.update(data, mac, iv)
        self.read_only = read_only

    def update(self, data: bytes, mac: bytes, iv: int) -> None:
        assert not self.read_only
        self.data = data
        self.mac = mac
        self.iv = iv


class Stream:
    PAGE_SIZE = 0x00000100
    PAGE_MASK = 0xffffff00
    PAGE_MASK_INVERT = (~PAGE_MASK & 0xffffffff)

    def __init__(self, app: App) -> None:
        self.pages: Dict[int, Page] = {}
        self.merkletree = MerkleTree()
        self.manifest = app.manifest
        self.signature = app.manifest_device_signature

        m = Manifest.from_binary(app.manifest)

        addr = m.code_start
        for data, mac in zip(app.code_pages, app.code_macs):
            assert addr not in self.pages
            # Since this pages are read-only, don't call _write_page() to avoid
            # inserting them in the merkle tree.
            self.pages[addr] = Page(data, mac, read_only=True)
            addr += Stream.PAGE_SIZE

        addr = m.data_start
        for data, mac in zip(app.data_pages, app.data_macs):
            assert addr not in self.pages
            # The IV is set to 0. It allows the VM to tell which key should be
            # used for decryption and HMAC verification.
            self._write_page(addr, data, mac, 0)
            addr += Stream.PAGE_SIZE

        self.send_buffer = b""
        self.send_buffer_counter = 0
        self.recv_buffer = b""
        self.recv_buffer_counter = 0

        self.client = get_client()

    def _get_page(self, addr: int) -> Page:
        assert (addr & Stream.PAGE_MASK_INVERT) == 0
        return self.pages[addr]

    def _write_page(self, addr: int, data: bytes, mac: bytes, iv: int) -> None:
        assert (addr & Stream.PAGE_MASK_INVERT) == 0
        assert len(data) == Stream.PAGE_SIZE
        if addr not in self.pages:
            self.pages[addr] = Page(data, mac, iv)
            self.merkletree.insert(Entry.from_values(addr, iv))
        else:
            self.pages[addr].update(data, mac, iv)
            self.merkletree.update(Entry.from_values(addr, iv))

    @staticmethod
    def _parse_request(struct: Struct, data: bytes) -> Any:
        assert len(data) == struct.sizeof()
        return struct.parse(data)

    def init_app(self) -> Apdu:
        apdu = exchange(self.client, ins=0x00, data=self.signature)
        assert apdu.status == 0x6701
        # pad with 3 bytes because of alignment
        data = b"\x00" * 3 + self.manifest
        return exchange(self.client, ins=0x00, data=data)

    def handle_read_access(self, data: bytes) -> Apdu:
        request = Stream._parse_request(Struct("addr" / Int32ul), data)
        # 1. addr was received
        logger.debug(f"read access: {request.addr:#x}")

        # 2. send encrypted page data
        page = self._get_page(request.addr)
        assert len(page.data) == Stream.PAGE_SIZE
        apdu = exchange(self.client, 0x01, data=page.data[1:], p2=page.data[0])
        assert apdu.status == 0x6102

        # 4. send iv and mac
        data = Struct("iv" / Int32ul, "mac" / Bytes(32)).build(dict(iv=page.iv, mac=page.mac))
        apdu = exchange(self.client, 0x02, data=data)

        # 5. send merkle proof
        if not page.read_only:
            assert apdu.status == 0x6103
            entry, proof = self.merkletree.get_proof(request.addr)
            assert entry.addr == request.addr
            assert entry.counter == page.iv

            # TODO: handle larger proofs
            assert len(proof) <= 250
            apdu = exchange(self.client, 0x03, data=proof)

        return apdu

    def handle_write_access(self, data: bytes) -> Apdu:
        # 1. encrypted page data was received
        assert len(data) == Stream.PAGE_SIZE
        page_data = data

        # 2. receive addr, iv and mac
        apdu = exchange(self.client, 0x01)
        assert apdu.status == 0x6202

        request = Stream._parse_request(Struct("addr" / Int32ul, "iv" / Int32ul, "mac" / Bytes(32)), apdu.data)
        assert (request.addr & Stream.PAGE_MASK_INVERT) == 0
        logger.debug(f"write access: {request.addr:#x} {request.iv:#x} {request.mac.hex()}")

        # 3. commit page and send merkle proof
        if self.merkletree.has_addr(request.addr):
            # proof of previous value
            entry, proof = self.merkletree.get_proof(request.addr)
            assert entry.addr == request.addr
            assert entry.counter + 1 == request.iv
        else:
            # proof of last entry
            entry, proof = self.merkletree.get_proof_of_last_entry()

        self._write_page(request.addr, page_data, request.mac, request.iv)

        # TODO: handle larger proofs
        assert len(proof) <= 250

        return exchange(self.client, 0x02, data=proof)

    def handle_send_buffer(self, data: bytes) -> Apdu:
        logger.info(f"got buffer {data}")

        request = Stream._parse_request(Struct("data" / Bytes(249), "size" / Int8ul, "counter" / Int32ul), data)

        stop = (request.counter & 0x80000000) != 0
        assert self.send_buffer_counter == (request.counter & 0x7fffffff)
        self.send_buffer_counter += 1

        assert request.size <= 249
        self.send_buffer += request.data[:request.size]

        if stop:
            logger.info(f"received buffer: {self.send_buffer} (len: {len(self.send_buffer)})")
            self.server.send_response(self.send_buffer)
            self.send_buffer = b""
            self.send_buffer_counter = 0

        return exchange(self.client, 0x00, data=b"")

    def handle_recv_buffer(self, data: bytes) -> Apdu:
        request = Stream._parse_request(Struct("counter" / Int32ul, "maxsize" / Int16ul), data)

        if len(self.recv_buffer) == 0:
            self.server = Server()
            self.recv_buffer = self.server.recv_request()

        assert self.recv_buffer_counter == request.counter
        self.recv_buffer_counter += 1

        buf = self.recv_buffer[:request.maxsize]
        self.recv_buffer = self.recv_buffer[request.maxsize:]
        print(f"buf: {buf}")
        print(f"buffer: {self.recv_buffer}")
        if len(self.recv_buffer) == 0:
            logger.debug(f"recv buffer last (size: {request.maxsize}, {len(buf)})")
            self.recv_buffer_counter = 0
            last = 0x01
        else:
            last = 0x00

        # the first byte is in p2
        if buf:
            p2 = buf[0]
            buf = buf[1:]
        else:
            p2 = 0x00

        return exchange(self.client, 0x00, p1=last, p2=p2, data=buf)

    def handle_exit(self, data: bytes) -> None:
        request = Stream._parse_request(Struct("code" / Int32ul), data)
        logger.warn(f"app exited with code {request.code}")

    def handle_fatal(self, data: bytes) -> None:
        request = Stream._parse_request(Struct("message" / Bytes(254)), data)
        message = request.message.rstrip(b"\x00")
        logger.warn(f"app encountered a fatal error: {message}")


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("stream")

    parser = argparse.ArgumentParser(description="RISC-V vm companion.")
    parser.add_argument("--app", type=str, required=True, help="encrypted application path (.zip)")
    parser.add_argument("--speculos", action="store_true", help="use speculos")
    parser.add_argument("--verbose", action="store_true", help="")

    args = parser.parse_args()

    if args.verbose:
        logger.setLevel(logging.DEBUG)

    import_ledgerwallet(args.speculos)

    if zipfile.is_zipfile(args.app):
        zip_path = args.app
        app = App.from_zip(zip_path)
    else:
        logger.warn("app is an ELF file... retrieving the HSM signature")
        zip_path = "/tmp/app.zip"
        app = hsm_sign_app(args.app)
        app.export_zip(zip_path)

    if app.manifest_device_signature is None:
        logger.warn("making the device sign the app")
        device_sign_app(app)
        app.export_zip(zip_path)

    stream = Stream(app)
    apdu = stream.init_app()

    while True:
        logger.debug(f"[<] {apdu.status:#06x} {apdu.data[:8].hex()}...")
        if apdu.status == 0x6101:
            apdu = stream.handle_read_access(apdu.data)
        elif apdu.status == 0x6201:
            apdu = stream.handle_write_access(apdu.data)
        elif apdu.status == 0x6301:
            apdu = stream.handle_send_buffer(apdu.data)
        elif apdu.status == 0x6401:
            apdu = stream.handle_recv_buffer(apdu.data)
        elif apdu.status == 0x6501:
            stream.handle_exit(apdu.data)
            break
        elif apdu.status == 0x6601:
            stream.handle_fatal(apdu.data)
            break
        else:
            logger.error(f"unexpected status {apdu.status:#06x}, {apdu.data}")
            assert False
