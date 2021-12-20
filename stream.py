#!/usr/bin/env python3

"""
https://github.com/LedgerHQ/ledgerctl/blob/master/ledgerwallet/client.py
"""

import sys

import binascii
import os
import pytest
import sys
import time
from enum import IntEnum

from elftools.elf.elffile import ELFFile
#from elftools.elf import ElfFile32l

os.environ["LEDGER_PROXY_ADDRESS"] = "127.0.0.1"
os.environ["LEDGER_PROXY_PORT"] = "9999"

from ledgerwallet.client import LedgerClient, CommException
from ledgerwallet.crypto.ecc import PrivateKey
from ledgerwallet.params import Bip32Path
from ledgerwallet.transport import enumerate_devices
from ledgerwallet.utils import serialize

CLA = 0x12

class Ins(IntEnum):
    GET_PUBLIC_KEY    = 0x02
    SIGN_SSH_BLOB     = 0x04
    SIGN_GENERIC_HASH = 0x06
    SIGN_DIRECT_HASH  = 0x08
    GET_ECDH_SECRET   = 0x0A

class P1(IntEnum):
    FIRST             = 0x00
    NEXT              = 0x01
    LAST_MARKER       = 0x80
    LAST              = 0x81


class Curve(IntEnum):
    PRIME256          = 0x01
    CURVE25519        = 0x02
    INVALID_03        = 0x03
    PUBLIC_KEY_MARKER = 0x80


def get_client():
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
    def __init__(self, path):
        with open(path, "rb") as fp:
            self.data = fp.read()

        self.fp = open(path, "rb")
        self.elf = ELFFile(self.fp)

        assert self.elf.get_machine_arch() == "RISC-V"
        text = self.elf.get_section_by_name(".text")
        sh_offset = text['sh_offset']
        sh_size = text['sh_size']

        self.sections = {}
        for section in self.elf.iter_sections():
            offset = section['sh_offset']
            size = section['sh_size']
            print(f"{offset:#x} {size:#x}")
            self.sections[offset] = section.data()


    def get_page(self, addr):
        assert addr & 0xff == 0

        #for section_addr, data in self.sections.items():
        #    if addr >= section_addr and addr < section_addr + len(data):

        if addr == 0x00010000:
            text = self.elf.get_section_by_name(".text")
            sh_offset = text['sh_offset']
            sh_size = text['sh_size']
            offset = sh_offset & 0xffffff00
            page = self.data[offset:offset+256]
        else:
            assert False

        return page


if __name__ == "__main__":
    stream = Stream("../app/test")
    client = get_client()

    first = True
    while True:
        if first:
            status_word, data = exchange(client, 0x00)
            first = False
            continue

        print(f"[<] {data}")
        if status_word == 0x6100:
            assert len(data) == 4
            addr = int.from_bytes(data, "little")
            page = stream.get_page(addr)

            response = client.raw_exchange(page)
            status_word = int.from_bytes(response[-2:], "big")
            data = response[:-2]
        else:
            assert False
