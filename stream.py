#!/usr/bin/env python3

"""
https://github.com/LedgerHQ/ledgerctl/blob/master/ledgerwallet/client.py
"""

import argparse
import logging
import os
import sys

from collections import namedtuple
from elftools.elf.elffile import ELFFile

Section = namedtuple("Section", ["name", "addr", "size", "data"])


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


def get_client():
    CLA = 0x12
    devices = enumerate_devices()
    if len(devices) == 0:
        print("No Ledger device has been found.")
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

    def __init__(self, path):
        with open(path, "rb") as fp:
            self.data = fp.read()

        self.fp = open(path, "rb")
        self.elf = ELFFile(self.fp)
        assert self.elf.get_machine_arch() == "RISC-V"

        self.sections = Stream._parse_sections(self.elf)

        self.stack = {}
        self.stack_end = 0x80000000
        self.stack_start = self.stack_end - 0x1000

    @staticmethod
    def _merge_sections(sections):
        """Merge adjacent sections."""

        # XXX: what if permissions are incompatible?

        if len(sections) < 2:
            return sections

        sections.sort(key=lambda s: s.addr)

        modified = True
        while modified:
            modified = False
            for i in range(0, len(sections)-1):
                a = sections[i]
                b = sections[i+1]

                a_end = a.addr + a.size
                assert a_end <= b.addr
                if (a_end & Stream.PAGE_MASK) == (b.addr & Stream.PAGE_MASK):
                    gap = b.addr - a_end
                    c = Section(f"{a.name}/{b.name}", a.addr, a.size + gap + b.size, a.data + (b"\xb5" * gap) + b.data)
                    sections[i] = c
                    sections.pop(i+1)
                    modified = True
                    break

        return sections

    @staticmethod
    def _pad_sections(sections):
        padded = []
        mask = Stream.PAGE_MASK_INVERT
        for s in sections:
            gap_start = s.addr & mask
            gap_end = (mask - ((s.addr + s.size) & mask)) & mask
            data = (b"\xc5" * gap_start) + s.data + (b"\xd5" * gap_end)
            section = Section(f"{s.name}[padded]", s.addr & Stream.PAGE_MASK, s.size + gap_start + gap_end, data)
            padded.append(section)
        return padded

    @staticmethod
    def _parse_sections(elf):
        sections = []
        for section in elf.iter_sections():
            if section.name not in [".text", ".rodata", ".data", ".sdata", ".sbss", ".bss"]:
                continue

            s = Section(section.name, section["sh_addr"], section["sh_size"], section.data())
            if s.size > 0:
                sections.append(s)

        sections = Stream._merge_sections(sections)
        sections = Stream._pad_sections(sections)

        for section in sections:
            logger.debug(f"section {section.name:10s}: {section.addr:#010x} - {section.addr+section.size:#010x} ({section.size} bytes)")

        return sections

    def get_page(self, addr):
        assert (addr & Stream.PAGE_MASK_INVERT) == 0

        page = None
        if addr >= self.stack_start and addr < self.stack_end:
            page = self.stack.get(addr, b"\x00" * 256)
        else:
            for section in self.sections:
                if addr >= section.addr and addr <= section.addr + section.size:
                    offset = (addr & Stream.PAGE_MASK) - section.addr
                    page = section.data[offset:offset+Stream.PAGE_SIZE]
                    break

        assert page is not None
        return page


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("stream")
    logger.setLevel(logging.DEBUG)

    parser = argparse.ArgumentParser(description="RISC-V vm companion.")
    parser.add_argument("--app", type=str, required=True, help="application path")
    parser.add_argument("--speculos", action="store_true", help="use speculos")

    args = parser.parse_args()

    import_ledgerwallet(args.speculos)

    stream = Stream(args.app)
    client = get_client()

    first = True
    while True:
        if first:
            entrypoint = stream.elf.header["e_entry"].to_bytes(4, "little")
            sp = int(stream.stack_end - 4).to_bytes(4, "little")
            status_word, data = exchange(client, ins=0x00, data=entrypoint + sp)
            first = False
            continue

        print(f"[<] {data}")
        if status_word == 0x6101:
            assert len(data) == 4
            addr = int.from_bytes(data, "little")
            print(f"[*] read access: {addr:#x}")
            page = stream.get_page(addr)

            response = client.raw_exchange(page)
            status_word = int.from_bytes(response[-2:], "big")
            data = response[:-2]
        elif status_word == 0x6102:
            assert len(data) == 4
            addr = int.from_bytes(data, "little")
            print(f"[*] write access: {addr:#x}")

            response = client.raw_exchange(bytes([0x01]))
            print(f"{response}")
            # XXX not alwasy stack
            stream.stack[addr] = response

            response = client.raw_exchange(bytes([0x02]))
            status_word = int.from_bytes(response[-2:], "big")
            data = response[:-2]
        elif data == b"":
            break
        else:
            print(f"{status_word:#06x}, {data}")
            assert False
