import hashlib
import hmac
import logging
import secrets

from elf import Elf, Segment
from merkletree import Entry, MerkleTree

from collections import namedtuple
from Crypto.Cipher import AES


logger = logging.getLogger("encryption")


EncryptedPage = namedtuple("EncryptedPage", "addr data mac")


class Encryption:
    """Encrypt and authenticate the code of an app."""

    def __init__(self):
        """
        Generates random static AES and HMAC keys.

        These (symmetric) secret keys are shared with the VM using the app
        manifest.
        """

        self.hmac_key = secrets.token_bytes(32)
        self.encryption_key = secrets.token_bytes(32)

    def _hmac(self, addr, data, iv):
        msg = data + addr.to_bytes(4, "little") + iv.to_bytes(4, "little")
        return hmac.new(self.hmac_key, msg, digestmod=hashlib.sha256).digest()

    def _encrypt(self, data, addr, iv):
        iv = addr.to_bytes(4, "little") + int(iv).to_bytes(4, "little")
        iv = iv.ljust(16, b"\x00")

        aes = AES.new(self.encryption_key, AES.MODE_CBC, iv)
        return aes.encrypt(data)

    def encrypt_segment(self, segment, page_size):
        pages = []
        iv = 0

        addr = segment.start
        for offset in range(0, segment.size, page_size):
            page = segment.data[offset:offset+page_size]
            assert len(page) == page_size

            encrypted_page = self._encrypt(page, addr, iv)
            assert len(encrypted_page) == page_size

            mac = self._hmac(addr, encrypted_page, iv)

            pages.append(EncryptedPage(addr, encrypted_page, mac))
            addr += page_size

        return pages


class EncryptedApp:
    def __init__(self, path):
        elf = Elf(path)
        enc = Encryption()

        self.code_pages = EncryptedApp._get_encrypted_pages(enc, elf, "code")
        self.data_pages = EncryptedApp._get_encrypted_pages(enc, elf, "data")
        self.manifest = EncryptedApp._get_manifest(enc, elf, self.data_pages)

    @staticmethod
    def _get_encrypted_pages(enc, elf, name):
        segment = elf.get_segment(name)
        pages = enc.encrypt_segment(segment, Segment.PAGE_SIZE)
        return pages

    @staticmethod
    def compute_initial_merkle_tree(elf, addresses):
        pages = []
        merkletree = MerkleTree()
        iv = 0

        # ensure addresses are sorted to compute the merkletree deterministically
        for addr in sorted(addresses):
            assert addr not in pages
            merkletree.insert(Entry.from_values(addr, iv))
            pages.append(addr)

        return merkletree

    @staticmethod
    def _get_manifest(enc, elf, data_pages):
        sdata_start, sdata_end = elf.get_section_range("data")
        scode_start, scode_end = elf.get_section_range("code")
        app_name = elf.app_infos["name"]
        assert len(app_name) == 32

        stack_end = 0x80000000
        stack_start = stack_end - Elf.STACK_SIZE
        bss = sdata_end

        addresses = [
            elf.entrypoint,
            bss,
            scode_start,
            scode_end,
            stack_start,
            stack_end,
            sdata_start,
            sdata_end + Elf.HEAP_SIZE,
        ]

        logger.debug(f"bss:   {bss:#x}")
        logger.debug(f"code:  {scode_start:#x} - {scode_end:#x}")
        logger.debug(f"data:  {sdata_start:#x} - {sdata_end + Elf.HEAP_SIZE:#x}")
        logger.debug(f"stack: {stack_start:#x} - {stack_end:#x}")

        data_addresses = [page.addr for page in data_pages]
        merkletree = EncryptedApp.compute_initial_merkle_tree(elf, data_addresses)

        data = b""
        data += app_name
        data += enc.hmac_key
        data += enc.encryption_key
        data += b"".join([addr.to_bytes(4, "little") for addr in addresses])
        data += merkletree.root_hash()
        data += len(merkletree.entries).to_bytes(4, "little")
        data += bytes(merkletree.entries[-1])

        return data
