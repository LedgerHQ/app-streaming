import hashlib
import hmac
import json
import logging
import secrets

from elf import Elf, Segment
from merkletree import Entry, MerkleTree

from collections import namedtuple
from Crypto.Cipher import AES
from zipfile import ZipFile


logger = logging.getLogger("encryption")


EncryptedPage = namedtuple("EncryptedPage", "addr data mac")


class Encryption:
    """Encrypt and authenticate the pages (code and data) of an app."""

    def __init__(self, hmac_key=None, encryption_key=None):
        """
        Generates random static AES and HMAC keys.

        These (symmetric) secret keys are shared with the VM using the app
        manifest.
        """

        if hmac_key is None and encryption_key is None:
            self.hmac_key = secrets.token_bytes(32)
            self.encryption_key = secrets.token_bytes(32)
        else:
            assert hmac_key is not None
            assert encryption_key is not None
            self.hmac_key = hmac_key
            self.encryption_key = encryption_key

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


class Manifest:
    """
    The manifest embeds info required by the VM to execute an app.
    """

    def __init__(self, enc, name, version, merkletree, entrypoint, code_start, code_size, data_start, data_size):
        self.enc = enc
        self.name = name
        self.version = version
        self.merkletree = merkletree
        self.entrypoint = entrypoint
        self.code_start = code_start
        self.code_size = code_size
        self.data_start = data_start
        self.data_size = data_size

    def export_binary(self):
        assert len(self.name) == 32

        stack_end = 0x80000000
        stack_start = stack_end - Elf.STACK_SIZE
        bss = self.data_start + self.data_size

        addresses = [
            self.entrypoint,
            bss,
            self.code_start,
            self.code_start + self.code_size,
            stack_start,
            stack_end,
            self.data_start,
            self.data_start + self.data_size + Elf.HEAP_SIZE,
        ]

        data = b""
        data += self.name
        data += self.enc.hmac_key
        data += self.enc.encryption_key
        data += b"".join([addr.to_bytes(4, "little") for addr in addresses])
        data += self.merkletree.root_hash()
        data += len(self.merkletree.entries).to_bytes(4, "little")
        data += bytes(self.merkletree.entries[-1])

        return data

    def export_json(self):
        """
        Export public information from a manifest file as JSON.

        These information are required to load an app from a zip file.
        """
        manifest = {
            # the name and version could be omitted but are convenient
            "name": self.name.decode("ascii").rstrip("\x00"),
            "version": self.name.decode("ascii").rstrip("\x00"),
            "code_start": self.code_start,
            "data_start": self.data_start,
        }
        return json.dumps(manifest, indent=4)


class EncryptedApp:
    def __init__(self, path):
        elf = Elf(path)
        enc = Encryption()

        self.code_pages = EncryptedApp._get_encrypted_pages(enc, elf, "code")
        self.data_pages = EncryptedApp._get_encrypted_pages(enc, elf, "data")

        manifest = EncryptedApp._get_manifest(enc, elf, self.data_pages)
        self.binary_manifest = manifest.export_binary()
        self.json_manifest = manifest.export_json()

    @classmethod
    def from_zip(cls, zip_path):
        def load_pages(addr, data):
            pages = []

            assert len(data) % (Segment.PAGE_SIZE + 32) == 0

            for offset in range(0, len(data), Segment.PAGE_SIZE + 32):
                page_data = data[offset:offset+Segment.PAGE_SIZE]
                mac = data[offset+Segment.PAGE_SIZE:offset+Segment.PAGE_SIZE+32]

                page = EncryptedPage(addr, page_data, mac)
                pages.append(page)

                addr += Segment.PAGE_SIZE

            return pages

        app = cls.__new__(cls)
        with ZipFile(zip_path, "r") as zf:
            m = json.loads(zf.read("manifest.json"))
            app.code_pages = load_pages(m["code_start"], zf.read("code.bin"))
            app.data_pages = load_pages(m["data_start"], zf.read("data.bin"))
            app.binary_manifest = zf.read("manifest.bin")
            app.json_manifest = zf.read("manifest.json")

        return app

    @staticmethod
    def _get_encrypted_pages(enc, elf, name):
        segment = elf.get_segment(name)
        pages = enc.encrypt_segment(segment, Segment.PAGE_SIZE)
        return pages

    @staticmethod
    def compute_initial_merkle_tree(addresses):
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
        code_start, code_end = elf.get_section_range("code")
        data_start, data_end = elf.get_section_range("data")
        code_size = code_end - code_start
        data_size = data_end - data_start
        name = elf.app_infos["name"]
        version = elf.app_infos["version"]
        assert len(name) == 32

        data_addresses = [page.addr for page in data_pages]
        merkletree = EncryptedApp.compute_initial_merkle_tree(data_addresses)

        return Manifest(enc, name, version, merkletree, elf.entrypoint, code_start, code_size, data_start, data_size)

    def export_zip(self, zip_path):
        def export_pages(pages):
            return b"".join([page.data + page.mac for page in pages])

        with ZipFile(zip_path, "w") as zf:
            zf.writestr("manifest.json", self.json_manifest)
            zf.writestr("manifest.bin", self.binary_manifest)
            zf.writestr("code.bin", export_pages(self.code_pages))
            zf.writestr("data.bin", export_pages(self.data_pages))
