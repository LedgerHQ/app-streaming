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
        self.stack = {}
        self.stack_end = 0x80000000
        self.stack_start = self.stack_end - 0x1000

        assert self.elf.get_machine_arch() == "RISC-V"

        text = self.elf.get_section_by_name(".text")
        self.code_start = text['sh_addr']
        self.code_end = self.code_start + text['sh_size']

        offset = self.code_start & 0xff
        if offset != 0:
            self.code_start -= offset

        offset_start = text['sh_offset'] - offset
        offset_end = text['sh_offset'] + text['sh_size']
        if offset_end & 0xff:
            offset_end = (offset_end + 0x100) & 0xffffff00
        self.code = self.data[offset_start:offset_end]
        #print(hex(offset_start), hex(offset_end))

    def get_page(self, addr):
        assert addr & 0xff == 0

        if addr >= self.code_start and addr < self.code_end:
            offset = (addr & 0xffffff00) - self.code_start
            page = self.code[offset:offset+256]
        elif addr >= self.stack_start and addr < self.stack_end:
            page = self.stack.get(addr, b"\x00" * 256)
        else:
            assert False

        return page


if __name__ == "__main__":
    stream = Stream("../app/test")
    client = get_client()

    first = True
    while True:
        if first:
            entrypoint = stream.elf.header["e_entry"].to_bytes(4, "little")
            #entrypoint = int(0x00010110).to_bytes(4, "little")
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
            stream.stack[addr] = response

            response = client.raw_exchange(bytes([0x02]))
            status_word = int.from_bytes(response[-2:], "big")
            data = response[:-2]
        else:
            assert False
