#!/usr/bin/env python3

"""
https://github.com/LedgerHQ/ledgerctl/blob/master/ledgerwallet/client.py
"""

import argparse
import logging
import os
import sys

from elftools.elf.elffile import ELFFile


class Section:
    def __init__(self, name, addr, size, data, writeable):
        self.name = name
        self.addr = addr
        self.size = size
        self.data = data
        self.writeable = writeable

    def get_page(self, addr):
        offset = (addr & Stream.PAGE_MASK) - self.addr
        return self.data[offset:offset+Stream.PAGE_SIZE]

    def write_page(self, addr, data):
        assert self.writeable

        offset = (addr & Stream.PAGE_MASK) - self.addr
        self.data = self.data[:offset] + data + self.data[offset+Stream.PAGE_SIZE:]


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
                    assert a.writeable == b.writeable
                    gap = b.addr - a_end
                    c = Section(f"{a.name}/{b.name}", a.addr, a.size + gap + b.size, a.data + (b"\xb5" * gap) + b.data, a.writeable)
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
            gap_end = (Stream.PAGE_SIZE - ((s.addr + s.size) & mask)) & mask
            data = (b"\xc5" * gap_start) + s.data + (b"\xd5" * gap_end)
            assert (len(data) % Stream.PAGE_SIZE) == 0
            section = Section(f"{s.name}[padded]", s.addr & Stream.PAGE_MASK, s.size + gap_start + gap_end, data, s.writeable)
            padded.append(section)
        return padded

    @staticmethod
    def _parse_sections(elf):
        sections = []
        permissions = {
            ".text": False,
            ".rodata": False,
            ".data": True,
            ".sdata": True,
            ".sbss": True,
            ".bss": True,
        }
        for section in elf.iter_sections():
            if section.name not in permissions.keys():
                continue

            writeable = permissions[section.name]
            s = Section(section.name, section["sh_addr"], section["sh_size"], section.data(), writeable)

            if section.name == ".bss":
                heap_size = 0x10000
                s.size += heap_size
                s.data += heap_size * b"\x00"

            if s.size > 0:
                sections.append(s)

        sections = Stream._merge_sections(sections)
        sections = Stream._pad_sections(sections)

        for section in sections:
            logger.debug(f"section {section.name:10s}: {section.addr:#010x} - {section.addr+section.size:#010x} ({section.size} bytes)")

        return sections

    def _get_section(self, addr):
        for section in self.sections:
            if addr >= section.addr and addr <= section.addr + section.size:
                return section

        assert False

    def _get_page(self, addr):
        assert (addr & Stream.PAGE_MASK_INVERT) == 0

        if addr >= self.stack_start and addr < self.stack_end:
            page = self.stack.get(addr, b"\x00" * 256)
        else:
            section = self._get_section(addr)
            page = section.get_page(addr)

        return page

    def _write_page(self, addr, data):
        assert (addr & Stream.PAGE_MASK_INVERT) == 0

        if addr >= self.stack_start and addr < self.stack_end:
            self.stack[addr] = data
        else:
            section = self._get_section(addr)
            section.write_page(addr, data)

    def init_app(self):
        entrypoint = self.elf.header["e_entry"]
        sp = self.stack_end - 4
        section_data = [s for s in self.sections if ".bss" in s.name]
        section_code = [s for s in self.sections if ".text" in s.name]
        assert len(section_data) == 1
        assert len(section_code) == 1
        addresses = [
            entrypoint,
            sp,
            section_code[0].addr,
            section_code[0].addr + section_code[0].size,
            section_data[0].addr,
            section_data[0].addr + section_data[0].size,
            self.stack_start,
            self.stack_end
        ]
        data = b"\x00" * 3  # for alignment
        data += b"".join([addr.to_bytes(4, "little") for addr in addresses])
        status_word, data = exchange(client, ins=0x00, data=data)
        return status_word, data

    def handle_read_access(self, data):
        assert len(data) == 4
        addr = int.from_bytes(data, "little")
        print(f"[*] read access: {addr:#x}")
        page = self._get_page(addr)
        assert len(page) == Stream.PAGE_SIZE

        status_word, data = exchange(client, 0x00, data=page[1:], p2=page[0])
        return status_word, data

    def handle_write_access(self, data, status_word):
        assert len(data) == 4 + (Stream.PAGE_SIZE - 2)

        addr = int.from_bytes(data, "little") & Stream.PAGE_MASK
        print(f"[*] write access: {addr:#x}")

        data = bytes([data[0]]) + data[4:] + bytes([status_word & 0xff])
        self._write_page(addr, data)

        status_word, data = exchange(client, 0x01)
        return status_word, data


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("stream")
    # logger.setLevel(logging.DEBUG)

    parser = argparse.ArgumentParser(description="RISC-V vm companion.")
    parser.add_argument("--app", type=str, required=True, help="application path")
    parser.add_argument("--speculos", action="store_true", help="use speculos")

    args = parser.parse_args()

    import_ledgerwallet(args.speculos)

    stream = Stream(args.app)
    client = get_client()

    status_word, data = stream.init_app()
    while True:
        print(f"[<] {status_word:#06x} {data[:4].hex()}")
        if status_word == 0x6101:
            status_word, data = stream.handle_read_access(data)
        elif (status_word & 0xff00) == 0x6200:
            status_word, data = stream.handle_write_access(data, status_word)
        else:
            print(f"unexpected status {status_word:#06x}, {data}")
            assert False
