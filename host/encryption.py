#!/usr/bin/env python3

import argparse
import hashlib
import hmac
import json
import logging
import secrets
import sys

from comm import exchange, get_client, import_ledgerwallet
from elf import Elf, Segment
from merkletree import Entry, MerkleTree

from construct import Bytes, Int32ul, Struct
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes, serialization
from Crypto.Cipher import AES
from typing import List, Tuple, Type
from zipfile import ZipFile


# >>> privkey = ec.generate_private_key(ec.SECP256K1(), default_backend())
# >>> hex(privkey.private_numbers().private_value)
PRIVATE_KEY_BYTES = bytes.fromhex("893d0436b796d91c47d6c463ec49802f4b7c6bbae03fdba0fe70d1c57da7056c")


def get_hsm_pubkey() -> bytes:
    private_key = ec.derive_private_key(int.from_bytes(PRIVATE_KEY_BYTES, "big"), ec.SECP256K1(), default_backend())
    public_key = private_key.public_key()
    format = serialization.PublicFormat.UncompressedPoint
    encoding = serialization.Encoding.X962
    return public_key.public_bytes(encoding=encoding, format=format)


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

    MANIFEST_STRUCT = Struct(
        "name" / Bytes(32),
        "version" / Bytes(16),
        "hmac_key" / Bytes(32),
        "encryption_key" / Bytes(32),
        "entrypoint" / Int32ul,
        "bss" / Int32ul,
        "code_start" / Int32ul,
        "code_end" / Int32ul,
        "stack_start" / Int32ul,
        "stack_end" / Int32ul,
        "data_start" / Int32ul,
        "data_end" / Int32ul,
        "mt_root_hash" / Bytes(32),
        "mt_size" / Int32ul,
        "mt_last_entry" / Bytes(8),
        "padding" / Bytes(4),
    )

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

    @staticmethod
    def encrypt_and_sign(data: bytes, pubkey: bytes) -> Tuple[bytes, bytes]:
        """
        Encrypt the manifest of an app using the public key of the device.
        """
        peer = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256K1(), pubkey)
        private_key = ec.derive_private_key(int.from_bytes(PRIVATE_KEY_BYTES, "big"), ec.SECP256K1(), default_backend())

        secret = private_key.exchange(ec.ECDH(), peer)
        secret_key = hashlib.sha256(secret).digest()
        iv = b"\x00" * 16
        aes = AES.new(secret_key, AES.MODE_CBC, iv)

        header, body = data[:32+16], data[32+16:]  # XXX
        encrypted_body = aes.encrypt(body)
        encrypted_manifest = header + encrypted_body

        signature = private_key.sign(encrypted_manifest, ec.ECDSA(hashes.SHA256()))

        return encrypted_manifest, signature

    def export_encrypted(self, device_pubkey: bytes) -> Tuple[bytes, bytes]:
        stack_end = 0x80000000
        stack_start = stack_end - Elf.STACK_SIZE
        bss = self.data_start + self.data_size

        data = Manifest.MANIFEST_STRUCT.build(dict({
            "name": self.name,
            "version": self.version,
            "hmac_key": self.enc.hmac_key,
            "encryption_key": self.enc.encryption_key,
            "entrypoint": self.entrypoint,
            "bss": bss,
            "code_start": self.code_start,
            "code_end": self.code_start + self.code_size,
            "stack_start": stack_start,
            "stack_end": stack_end,
            "data_start": self.data_start,
            "data_end": self.data_start + self.data_size + Elf.HEAP_SIZE,
            "mt_root_hash": self.merkletree.root_hash(),
            "mt_size": len(self.merkletree.entries),
            "mt_last_entry": bytes(self.merkletree.entries[-1]),
            "padding": b"\x00" * 4,
        }))

        assert len(data) % AES.block_size == 0

        return Manifest.encrypt_and_sign(data, device_pubkey)

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
    def __init__(self, path: str, device_pubkey: bytes) -> None:
        elf = Elf(path)
        enc = Encryption()

        self.code_pages = EncryptedApp._get_encrypted_pages(enc, elf, "code")
        self.data_pages = EncryptedApp._get_encrypted_pages(enc, elf, "data")

        manifest = EncryptedApp._get_manifest(enc, elf, self.data_pages)
        self.binary_manifest, self.binary_manifest_signature = manifest.export_encrypted(device_pubkey)
        self.json_manifest = manifest.export_json()

    @classmethod
    def from_zip(cls: Type[object], zip_path: str):
        app = cls.__new__(cls)
        with ZipFile(zip_path, "r") as zf:
            m = json.loads(zf.read("manifest.json"))
            app.code_pages = EncryptedPage.load(m["code_start"], zf.read("code.bin"))
            app.data_pages = EncryptedPage.load(m["data_start"], zf.read("data.bin"))
            app.binary_manifest = zf.read("manifest.bin")
            app.binary_manifest_signature = zf.read("manifest.sig.bin")
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
            zf.writestr("manifest.sig.bin", self.binary_manifest_signature)
            zf.writestr("code.bin", EncryptedPage.export(self.code_pages))
            zf.writestr("data.bin", EncryptedPage.export(self.data_pages))


def get_device_pubkey(output_pubkey_file=None) -> bytes:
    client = get_client()

    name = b"a" * 32
    version = b"b" * 16
    data = name + version
    apdu = exchange(client, ins=0x10, data=data, cla=0x34)
    assert apdu.status == 0x9000
    pubkey = apdu.data

    if output_pubkey_file:
        with open(output_pubkey_file, "wb") as fp:
            fp.write(pubkey)

    return pubkey


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("encryption")

    parser = argparse.ArgumentParser(description="Tool to encrypt app ELF binaries.")
    parser.add_argument("--app-file", type=str, help="application path (.elf)")
    parser.add_argument("--output-file", type=str, help="path to the encrypted app (.zip)")
    parser.add_argument("--output-pubkey-file", type=str, help="write the device public key path (.der)")
    parser.add_argument("--pubkey-file", type=str, help="device public key path (.der)")
    parser.add_argument("--print-hsm-pubkey", action="store_true", help="print HSM public key")
    parser.add_argument("--speculos", action="store_true", help="use speculos")

    args = parser.parse_args()

    if args.print_hsm_pubkey:
        print(f"{get_hsm_pubkey().hex()}")
        sys.exit(0)

    if not args.app_file or not args.output_file:
        logger.error("--app-file and --output-file are required")
        sys.exit(1)

    if args.pubkey_file:
        if args.output_pubkey_file:
            logger.error("it doesn't make sense to use --pubkey-file with --output-pubkey-file")
            sys.exit(1)

        with open(args.pubkey_file, "rb") as fp:
            pubkey = fp.read()
    else:
        import_ledgerwallet(args.speculos)
        pubkey = get_device_pubkey(args.output_pubkey_file)

    app = EncryptedApp(args.app_file, pubkey)
    app.export_zip(args.output_file)

    # ensure that the generated file is valid
    EncryptedApp.from_zip(args.output_file)
