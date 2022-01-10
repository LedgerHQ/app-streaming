#!/usr/bin/env python3

"""
https://github.com/LedgerHQ/ledgerctl/blob/master/ledgerwallet/client.py
"""

import argparse
import logging
import os
import sys

from elf import Elf
from merkletree import Entry, MerkleTree


class Page:
    def __init__(self, data, mac, iv=0, read_only=False):
        self.read_only = False
        self.update(data, mac, iv)
        self.read_only = read_only

    def update(self, data, mac, iv):
        assert not self.read_only
        self.data = data
        self.mac = mac
        self.iv = iv


def import_ledgerwallet(use_speculos: bool) -> None:
    global LedgerClient
    global CommException
    global enumerate_devices
    global serialize

    if use_speculos:
        os.environ["LEDGER_PROXY_ADDRESS"] = "127.0.0.1"
        os.environ["LEDGER_PROXY_PORT"] = "9999"

    from ledgerwallet.client import LedgerClient, CommException
    from ledgerwallet.transport import enumerate_devices
    from ledgerwallet.utils import serialize

    if False:
        logger = logging.getLogger("ledgerwallet")
        logger.setLevel(logging.DEBUG)


def get_client():
    CLA = 0x12
    devices = enumerate_devices()
    if len(devices) == 0:
        logger.error("No Ledger device has been found.")
        sys.exit(0)

    return LedgerClient(devices[0], cla=CLA)


def exchange(client, ins, data=b"", p1=0, p2=0):
    apdu = bytes([client.cla, ins, p1, p2])
    apdu += serialize(data)
    response = client.raw_exchange(apdu)
    status_word = int.from_bytes(response[-2:], "big")
    return status_word, response[:-2]


class Stream:
    PAGE_SIZE = 0x00000100
    PAGE_MASK = 0xffffff00
    PAGE_MASK_INVERT = (~PAGE_MASK & 0xffffffff)

    HEAP_SIZE = 0x10000
    STACK_SIZE = 0x1000

    def __init__(self, path):
        self.elf = Elf(path)

        self.pages = {}
        self.merkletree = MerkleTree()

        for addr, (data, mac) in self.elf.get_encrypted_pages("code"):
            assert addr not in self.pages
            # Since this pages are read-only, don't call _write_page() to avoid
            # inserting them in the merkle tree.
            self.pages[addr] = Page(data, mac, read_only=True)

        iv = 1
        for addr, (data, mac) in self.elf.get_encrypted_pages("data", iv):
            assert addr not in self.pages
            # The IV is set to 1. This is an error for the VM to encounter
            # writeable pages initialized to 0.
            self._write_page(addr, data, mac, iv)

        self.stack = {}
        self.stack_end = 0x80000000
        self.stack_start = self.stack_end - Stream.STACK_SIZE

    def _get_page(self, addr):
        assert (addr & Stream.PAGE_MASK_INVERT) == 0
        return self.pages[addr]

    def _write_page(self, addr, data, mac, iv):
        assert (addr & Stream.PAGE_MASK_INVERT) == 0
        assert len(data) == Stream.PAGE_SIZE
        if addr not in self.pages:
            self.pages[addr] = Page(data, mac, iv)
            self.merkletree.insert(Entry.from_values(addr, iv))
        else:
            self.pages[addr].update(data, mac, iv)
            self.merkletree.update(Entry.from_values(addr, iv))

    def init_app(self):
        entrypoint = self.elf.entrypoint

        sdata_start, sdata_end = self.elf.get_section_range("data")
        scode_start, scode_end = self.elf.get_section_range("code")

        bss = sdata_end

        addresses = [
            entrypoint,
            bss,
            scode_start,
            scode_end,
            self.stack_start,
            self.stack_end,
            sdata_start,
            sdata_end + Stream.HEAP_SIZE,
        ]

        logger.debug(f"bss:   {bss:#x}")
        logger.debug(f"code:  {scode_start:#x} - {scode_end:#x}")
        logger.debug(f"data:  {sdata_start:#x} - {sdata_end + Stream.HEAP_SIZE:#x}")
        logger.debug(f"stack: {self.stack_start:#x} - {self.stack_end:#x}")

        data = b"\x00" * 3  # for alignment
        data += b"".join([addr.to_bytes(4, "little") for addr in addresses])
        data += self.merkletree.root_hash()
        data += len(self.merkletree.entries).to_bytes(4, "little")
        data += bytes(self.merkletree.entries[-1])
        status_word, data = exchange(client, ins=0x00, data=data)
        return status_word, data

    def handle_read_access(self, data):
        # 1. addr was received
        assert len(data) == 4
        addr = int.from_bytes(data, "little")
        logger.debug(f"read access: {addr:#x}")

        # 2. send encrypted page data
        page = self._get_page(addr)
        assert len(page.data) == Stream.PAGE_SIZE
        status_word, data = exchange(client, 0x01, data=page.data[1:], p2=page.data[0])
        assert status_word == 0x6102

        # 4. send iv and mac
        assert len(page.mac) == 32
        data = page.iv.to_bytes(4, "little") + page.mac
        status_word, data = exchange(client, 0x02, data=data)

        # 5. send merkle proof
        if not page.read_only:
            assert status_word == 0x6103
            entry, proof = self.merkletree.get_proof(addr)
            assert entry.addr == addr
            assert entry.counter == page.iv

            # TODO: handle larger proofs
            assert len(proof) <= 250
            status_word, data = exchange(client, 0x03, data=proof)

        return status_word, data

    def handle_write_access(self, data):
        # 1. encrypted page data was received
        assert len(data) == Stream.PAGE_SIZE
        page_data = data

        # 2. receive addr, iv and mac
        status_word, data = exchange(client, 0x01)
        assert status_word == 0x6202
        assert len(data) == 4 + 4 + 32

        addr = int.from_bytes(data[:4], "little") & Stream.PAGE_MASK
        iv = int.from_bytes(data[4:8], "little")
        mac = data[8:]
        logger.debug(f"write access: {addr:#x} {iv:#x} {mac.hex()}")

        # 3. commit page and send merkle proof
        if self.merkletree.has_addr(addr):
            # proof of previous value
            entry, proof = self.merkletree.get_proof(addr)
            assert entry.addr == addr
            assert entry.counter + 1 == iv
        else:
            # proof of last entry
            entry, proof = self.merkletree.get_proof_of_last_entry()

        self._write_page(addr, page_data, mac, iv)

        # TODO: handle larger proofs
        assert len(proof) <= 250

        status_word, data = exchange(client, 0x02, data=proof)
        return status_word, data


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
        else:
            logger.error(f"unexpected status {status_word:#06x}, {data}")
            assert False
