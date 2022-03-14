import hashlib
import hmac
import json
import logging
import secrets

from elf import Elf, Segment
from merkletree import Entry, MerkleTree

from Crypto.Cipher import AES
from typing import List, Type
from zipfile import ZipFile


logger = logging.getLogger("encryption")


class EncryptedPage:
    def __init__(self, addr: int, data: bytes, mac: bytes) -> None:
        self.addr = addr
        self.data = data
        self.mac = mac

    @staticmethod
    def load(addr: int, data: bytes) -> List["EncryptedPage"]:
        pages = []

        assert len(data) % (Segment.PAGE_SIZE + 32) == 0

        for offset in range(0, len(data), Segment.PAGE_SIZE + 32):
            page_data = data[offset:offset+Segment.PAGE_SIZE]
            mac = data[offset+Segment.PAGE_SIZE:offset+Segment.PAGE_SIZE+32]

            page = EncryptedPage(addr, page_data, mac)
            pages.append(page)

            addr += Segment.PAGE_SIZE

        return pages

    @staticmethod
    def export(pages: List["EncryptedPage"]) -> bytes:
        return b"".join([page.data + page.mac for page in pages])


class Encryption:
    """Encrypt and authenticate the pages (code and data) of an app."""

    def __init__(self) -> None:
        """
        Generates random static AES and HMAC keys.

        These (symmetric) secret keys are shared with the VM using the app
        manifest.
        """

        self.hmac_key = secrets.token_bytes(32)
        self.encryption_key = secrets.token_bytes(32)

    def _hmac(self, addr: int, data: bytes, iv: int) -> bytes:
        msg = data + addr.to_bytes(4, "little") + iv.to_bytes(4, "little")
        return hmac.new(self.hmac_key, msg, digestmod=hashlib.sha256).digest()

    def _encrypt(self, data: bytes, addr: int, iv: int) -> bytes:
        ivbin = addr.to_bytes(4, "little") + iv.to_bytes(4, "little")
        ivbin = ivbin.ljust(16, b"\x00")

        aes = AES.new(self.encryption_key, AES.MODE_CBC, ivbin)
        return aes.encrypt(data)

    def encrypt_segment(self, segment: Segment) -> List[EncryptedPage]:
        pages = []
        iv = 0

        page_size = Segment.PAGE_SIZE
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

    def __init__(self, enc: Encryption, name: bytes, version: bytes, merkletree: MerkleTree, entrypoint: int, code_start: int, code_size: int, data_start: int, data_size: int) -> None:
        self.enc = enc
        self.name = name
        self.version = version
        self.merkletree = merkletree
        self.entrypoint = entrypoint
        self.code_start = code_start
        self.code_size = code_size
        self.data_start = data_start
        self.data_size = data_size

    def export_encrypted(self, device_key: int):
        def encrypt_manifest(data, device_key):
            """XXX - TODO: temporary xor encryption until a correct scheme is found."""
            return bytes([c ^ device_key for c in data])

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

        return encrypt_manifest(data, device_key)

    def export_json(self) -> str:
        """
        Export public information from a manifest file as JSON.

        These information are required to load an app from a zip file.
        """
        manifest = {
            # the name and version could be omitted but are convenient
            "name": self.name.decode("ascii").rstrip("\x00"),
            "version": self.version.decode("ascii").rstrip("\x00"),
            "code_start": self.code_start,
            "data_start": self.data_start,
        }
        return json.dumps(manifest, indent=4)


class EncryptedApp:
    def __init__(self, path: str, device_key: int) -> None:
        elf = Elf(path)
        enc = Encryption()

        self.code_pages = EncryptedApp._get_encrypted_pages(enc, elf, "code")
        self.data_pages = EncryptedApp._get_encrypted_pages(enc, elf, "data")

        manifest = EncryptedApp._get_manifest(enc, elf, self.data_pages)
        self.binary_manifest = manifest.export_encrypted(device_key)
        self.json_manifest = manifest.export_json()

    @classmethod
    def from_zip(cls: Type[object], zip_path: str):
        app = cls.__new__(cls)
        with ZipFile(zip_path, "r") as zf:
            m = json.loads(zf.read("manifest.json"))
            app.code_pages = EncryptedPage.load(m["code_start"], zf.read("code.bin"))
            app.data_pages = EncryptedPage.load(m["data_start"], zf.read("data.bin"))
            app.binary_manifest = zf.read("manifest.bin")
            app.json_manifest = zf.read("manifest.json")

        return app

    @staticmethod
    def _get_encrypted_pages(enc: Encryption, elf: Elf, name: str):
        segment = elf.get_segment(name)
        pages = enc.encrypt_segment(segment)
        return pages

    @staticmethod
    def compute_initial_merkle_tree(addresses: List[int]) -> MerkleTree:
        pages: List[int] = []
        merkletree = MerkleTree()
        iv = 0

        # ensure addresses are sorted to compute the merkletree deterministically
        for addr in sorted(addresses):
            assert addr not in pages
            merkletree.insert(Entry.from_values(addr, iv))
            pages.append(addr)

        return merkletree

    @staticmethod
    def _get_manifest(enc: Encryption, elf: Elf, data_pages: List[EncryptedPage]) -> Manifest:
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

    def export_zip(self, zip_path: str) -> None:
        with ZipFile(zip_path, "w") as zf:
            zf.writestr("manifest.json", self.json_manifest)
            zf.writestr("manifest.bin", self.binary_manifest)
            zf.writestr("code.bin", EncryptedPage.export(self.code_pages))
            zf.writestr("data.bin", EncryptedPage.export(self.data_pages))
