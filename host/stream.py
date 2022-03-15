#!/usr/bin/env python3

"""
https://github.com/LedgerHQ/ledgerctl/blob/master/ledgerwallet/client.py
"""

import argparse
import logging
import os
import sys

from construct import Bytes, Int8ul, Int16ul, Int32ul, Struct
from encryption import EncryptedApp
from merkletree import Entry, MerkleTree
from typing import Any, Dict, Tuple
from server import Server

from ledgerwallet.client import LedgerClient
from ledgerwallet.transport import enumerate_devices
from ledgerwallet.utils import serialize


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


def import_ledgerwallet(use_speculos: bool) -> None:
    global LedgerClient
    global enumerate_devices
    global serialize

    if use_speculos:
        os.environ["LEDGER_PROXY_ADDRESS"] = "127.0.0.1"
        os.environ["LEDGER_PROXY_PORT"] = "9999"

    if False:
        logger = logging.getLogger("ledgerwallet")
        logger.setLevel(logging.DEBUG)


def get_client() -> LedgerClient:
    CLA = 0x12
    devices = enumerate_devices()
    if len(devices) == 0:
        logger.error("No Ledger device has been found.")
        sys.exit(0)

    return LedgerClient(devices[0], cla=CLA)


def exchange(client: LedgerClient, ins: int, data=b"", p1=0, p2=0) -> Tuple[int, bytes]:
    apdu = bytes([client.cla, ins, p1, p2])
    apdu += serialize(data)
    response = client.raw_exchange(apdu)
    status_word = int.from_bytes(response[-2:], "big")
    return status_word, response[:-2]


class Stream:
    PAGE_SIZE = 0x00000100
    PAGE_MASK = 0xffffff00
    PAGE_MASK_INVERT = (~PAGE_MASK & 0xffffffff)

    def __init__(self, path: str) -> None:
        device_key = 0x47
        app = EncryptedApp(path, device_key)

        if True:
            app.export_zip("/tmp/app.zip")
            app = EncryptedApp.from_zip("/tmp/app.zip")

        self.pages: Dict[int, Page] = {}
        self.merkletree = MerkleTree()
        self.manifest = app.binary_manifest

        for page in app.code_pages:
            assert page.addr not in self.pages
            # Since this pages are read-only, don't call _write_page() to avoid
            # inserting them in the merkle tree.
            self.pages[page.addr] = Page(page.data, page.mac, read_only=True)

        for page in app.data_pages:
            assert page.addr not in self.pages
            # The IV is set to 0. It allows the VM to tell which key should be
            # used for decryption and HMAC verification.
            self._write_page(page.addr, page.data, page.mac, 0)

        self.send_buffer = b""
        self.send_buffer_counter = 0
        self.recv_buffer = b""
        self.recv_buffer_counter = 0

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

    def init_app(self) -> Tuple[int, bytes]:
        data = b"\x00" * 3  # for alignment
        data += self.manifest
        status_word, data = exchange(client, ins=0x00, data=data)
        return status_word, data

    @staticmethod
    def _parse_request(struct: Struct, data: bytes) -> Any:
        assert len(data) == struct.sizeof()
        return struct.parse(data)

    def handle_read_access(self, data: bytes) -> Tuple[int, bytes]:
        request = Stream._parse_request(Struct("addr" / Int32ul), data)
        # 1. addr was received
        logger.debug(f"read access: {request.addr:#x}")

        # 2. send encrypted page data
        page = self._get_page(request.addr)
        assert len(page.data) == Stream.PAGE_SIZE
        status_word, data = exchange(client, 0x01, data=page.data[1:], p2=page.data[0])
        assert status_word == 0x6102

        # 4. send iv and mac
        data = Struct("iv" / Int32ul, "mac" / Bytes(32)).build(dict(iv=page.iv, mac=page.mac))
        status_word, data = exchange(client, 0x02, data=data)

        # 5. send merkle proof
        if not page.read_only:
            assert status_word == 0x6103
            entry, proof = self.merkletree.get_proof(request.addr)
            assert entry.addr == request.addr
            assert entry.counter == page.iv

            # TODO: handle larger proofs
            assert len(proof) <= 250
            status_word, data = exchange(client, 0x03, data=proof)

        return status_word, data

    def handle_write_access(self, data: bytes) -> Tuple[int, bytes]:
        # 1. encrypted page data was received
        assert len(data) == Stream.PAGE_SIZE
        page_data = data

        # 2. receive addr, iv and mac
        status_word, data = exchange(client, 0x01)
        assert status_word == 0x6202

        request = Stream._parse_request(Struct("addr" / Int32ul, "iv" / Int32ul, "mac" / Bytes(32)), data)
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

        status_word, data = exchange(client, 0x02, data=proof)
        return status_word, data

    def handle_send_buffer(self, data: bytes) -> Tuple[int, bytes]:
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

        status_word, data = exchange(client, 0x00, data=b"")
        return status_word, data

    def handle_recv_buffer(self, data: bytes) -> Tuple[int, bytes]:
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
        status_word, data = exchange(client, 0x00, p1=last, p2=p2, data=buf)
        return status_word, data

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
    parser.add_argument("--app", type=str, required=True, help="application path")
    parser.add_argument("--speculos", action="store_true", help="use speculos")
    parser.add_argument("--verbose", action="store_true", help="")

    args = parser.parse_args()

    if args.verbose:
        logger.setLevel(logging.DEBUG)

    import_ledgerwallet(args.speculos)

    stream = Stream(args.app)
    client = get_client()

    status_word, data = stream.init_app()
    while True:
        logger.debug(f"[<] {status_word:#06x} {data[:4].hex()}")
        if status_word == 0x6101:
            status_word, data = stream.handle_read_access(data)
        elif status_word == 0x6201:
            status_word, data = stream.handle_write_access(data)
        elif status_word == 0x6301:
            status_word, data = stream.handle_send_buffer(data)
        elif status_word == 0x6401:
            status_word, data = stream.handle_recv_buffer(data)
        elif status_word == 0x6501:
            stream.handle_exit(data)
            break
        elif status_word == 0x6601:
            stream.handle_fatal(data)
            break
        else:
            logger.error(f"unexpected status {status_word:#06x}, {data}")
            assert False
