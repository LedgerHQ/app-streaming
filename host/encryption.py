#!/usr/bin/env python3

import argparse
import hashlib
import hmac
import logging
import sys

from comm import exchange, get_client, import_ledgerwallet
from elf import Elf, Segment
from merkletree import Entry, MerkleTree

from construct import Bytes, Int8ul, Int32ul, Struct
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.ec import EllipticCurvePrivateKey, EllipticCurvePublicKey
from cryptography.hazmat.primitives import hashes, serialization
from Crypto.Cipher import AES
from typing import List, Type
from zipfile import ZipFile


class HSM:
    # >>> privkey = ec.generate_private_key(ec.SECP256K1(), default_backend())
    # >>> hex(privkey.private_numbers().private_value)
    PRIVATE_KEY_BYTES = bytes.fromhex("893d0436b796d91c47d6c463ec49802f4b7c6bbae03fdba0fe70d1c57da7056c")

    def __init__(self):
        pass

    @staticmethod
    def _get_private_key() -> EllipticCurvePrivateKey:
        private_value = int.from_bytes(HSM.PRIVATE_KEY_BYTES, "big")
        private_key = ec.derive_private_key(private_value, ec.SECP256K1(), default_backend())
        return private_key

    @staticmethod
    def get_pubkey() -> EllipticCurvePublicKey:
        private_key = HSM._get_private_key()
        public_key = private_key.public_key()
        return public_key

    @staticmethod
    def get_pubkey_bytes() -> bytes:
        format = serialization.PublicFormat.UncompressedPoint
        encoding = serialization.Encoding.X962
        return HSM.get_pubkey().public_bytes(encoding=encoding, format=format)

    @staticmethod
    def sign_manifest(data: bytes) -> bytes:
        private_key = HSM._get_private_key()
        signature = private_key.sign(data, ec.ECDSA(hashes.SHA256()))
        return signature

    @staticmethod
    def decrypt_device_keys(app_hash: bytes, device_keys: Struct) -> "Encryption":
        """
        Decrypt the HMAC and encryption keys sent by the device.

        The shared secret used to encrypt these keys is established through ECDH.
        """

        peer = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256K1(), device_keys.pubkey)

        signature = device_keys.signature[:device_keys.sig_size]
        peer.verify(signature, device_keys.encrypted_keys, ec.ECDSA(hashes.SHA256()))

        private_key = HSM._get_private_key()
        secret = private_key.exchange(ec.ECDH(), peer)
        secret_key = hashlib.sha256(secret).digest()

        iv = b"\x00" * 16
        aes = AES.new(secret_key, AES.MODE_CBC, iv)
        hmac_key = aes.decrypt(device_keys.encrypted_keys)

        return Encryption(peer, hmac_key)


class AuthenticatedPage:
    def __init__(self, addr: int, data: bytes, mac: bytes) -> None:
        self.addr = addr
        self.data = data
        self.mac = mac

    @staticmethod
    def load(addr: int, data: bytes) -> List["AuthenticatedPage"]:
        pages = []

        assert len(data) % (Segment.PAGE_SIZE + 32) == 0

        for offset in range(0, len(data), Segment.PAGE_SIZE + 32):
            page_data = data[offset:offset+Segment.PAGE_SIZE]
            mac = data[offset+Segment.PAGE_SIZE:offset+Segment.PAGE_SIZE+32]

            page = AuthenticatedPage(addr, page_data, mac)
            pages.append(page)

            addr += Segment.PAGE_SIZE

        return pages

    @staticmethod
    def export(pages: List["AuthenticatedPage"]) -> bytes:
        return b"".join([page.data + page.mac for page in pages])


class Encryption:
    """Encrypt and authenticate the pages (code and data) of an app."""

    def __init__(self, public_key: EllipticCurvePublicKey, hmac_key: bytes) -> None:
        self.public_key = public_key
        self.hmac_key = hmac_key

    def _hmac(self, addr: int, data: bytes, iv: int) -> bytes:
        msg = data + addr.to_bytes(4, "little") + iv.to_bytes(4, "little")
        return hmac.new(self.hmac_key, msg, digestmod=hashlib.sha256).digest()

    def encrypt_segment(self, segment: Segment) -> List[AuthenticatedPage]:
        pages = []
        iv = 0

        page_size = Segment.PAGE_SIZE
        addr = segment.start
        for offset in range(0, segment.size, page_size):
            page = segment.data[offset:offset+page_size]
            assert len(page) == page_size

            mac = self._hmac(addr, page, iv)

            pages.append(AuthenticatedPage(addr, page, mac))
            addr += page_size

        return pages


class Manifest:
    """
    The manifest embeds info required by the VM to execute an app.
    """

    MANIFEST_STRUCT = Struct(
        "name" / Bytes(32),
        "version" / Bytes(16),
        "app_hash" / Bytes(32),
        "pubkey_hash" / Bytes(32),
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
    )

    def __init__(self, name: bytes, version: bytes, app_hash: bytes, pubkey_hash: bytes, merkletree: MerkleTree, entrypoint: int, code_start: int, code_size: int, data_start: int, data_size: int) -> None:
        self.name = name
        self.version = version
        self.app_hash = app_hash
        self.pubkey_hash = pubkey_hash
        self.entrypoint = entrypoint
        self.code_start = code_start
        self.code_size = code_size
        self.data_start = data_start
        self.data_size = data_size

        self.mt_root_hash = merkletree.root_hash()
        self.mt_size = len(merkletree.entries)
        self.mt_last_entry = bytes(merkletree.entries[-1])

    @classmethod
    def from_binary(cls: Type[object], data: bytes) -> "Manifest":
        assert len(data) == Manifest.MANIFEST_STRUCT.sizeof()
        m = Manifest.MANIFEST_STRUCT.parse(data)

        manifest = cls.__new__(cls)
        manifest.name = m.name
        manifest.version = m.version
        manifest.app_hash = m.app_hash
        manifest.pubkey_hash = m.pubkey_hash
        manifest.entrypoint = m.entrypoint
        manifest.code_start = m.code_start
        manifest.code_size = m.code_end - m.code_start
        manifest.data_start = m.data_start
        manifest.data_size = m.data_end - m.data_start
        manifest.mt_root_hash = m.mt_root_hash
        manifest.mt_size = m.mt_size
        manifest.mt_last_entry = m.mt_last_entry

        return manifest

    def export_binary(self) -> bytes:
        stack_end = 0x80000000
        stack_start = stack_end - Elf.STACK_SIZE
        bss = self.data_start + self.data_size

        data = Manifest.MANIFEST_STRUCT.build(dict({
            "name": self.name,
            "version": self.version,
            "app_hash": self.app_hash,
            "pubkey_hash": self.pubkey_hash,
            "entrypoint": self.entrypoint,
            "bss": bss,
            "code_start": self.code_start,
            "code_end": self.code_start + self.code_size,
            "stack_start": stack_start,
            "stack_end": stack_end,
            "data_start": self.data_start,
            "data_end": self.data_start + self.data_size + Elf.HEAP_SIZE,
            "mt_root_hash": self.mt_root_hash,
            "mt_size": self.mt_size,
            "mt_last_entry": self.mt_last_entry,
        }))

        return data

    def __str__(self):
        return str(Manifest.MANIFEST_STRUCT.parse(self.export_binary()))


class EncryptedApp:
    def __init__(self, path: str, device_keys: bytes) -> None:
        elf = Elf(path)
        enc = HSM.decrypt_device_keys(elf.app_hash(), device_keys)

        self.code_pages = EncryptedApp._get_encrypted_pages(enc, elf, "code")
        self.data_pages = EncryptedApp._get_encrypted_pages(enc, elf, "data")

        manifest = EncryptedApp._get_manifest(elf, self.data_pages, enc.public_key)
        self.binary_manifest = manifest.export_binary()
        self.binary_manifest_signature = HSM.sign_manifest(self.binary_manifest)

    @classmethod
    def from_zip(cls: Type[object], zip_path: str):
        app = cls.__new__(cls)
        with ZipFile(zip_path, "r") as zf:
            app.binary_manifest = zf.read("manifest.bin")
            app.binary_manifest_signature = zf.read("manifest.bin.sig")

            pubkey = HSM.get_pubkey()
            pubkey.verify(app.binary_manifest_signature, app.binary_manifest, ec.ECDSA(hashes.SHA256()))

            manifest = Manifest.from_binary(app.binary_manifest)

            app.code_pages = AuthenticatedPage.load(manifest.code_start, zf.read("code.bin"))
            app.data_pages = AuthenticatedPage.load(manifest.data_start, zf.read("data.bin"))

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
    def _get_manifest(elf: Elf, data_pages: List[AuthenticatedPage], pubkey: EllipticCurvePublicKey) -> Manifest:
        code_start, code_end = elf.get_section_range("code")
        data_start, data_end = elf.get_section_range("data")
        code_size = code_end - code_start
        data_size = data_end - data_start
        name = elf.app_infos["name"]
        version = elf.app_infos["version"]
        assert len(name) == 32

        data_addresses = [page.addr for page in data_pages]
        merkletree = EncryptedApp.compute_initial_merkle_tree(data_addresses)

        format = serialization.PublicFormat.UncompressedPoint
        encoding = serialization.Encoding.X962
        pubkey_hash = hashlib.sha256(pubkey.public_bytes(encoding=encoding, format=format)).digest()

        return Manifest(name, version, elf.app_hash(), pubkey_hash, merkletree, elf.entrypoint, code_start, code_size, data_start, data_size)

    def export_zip(self, zip_path: str) -> None:
        with ZipFile(zip_path, "w") as zf:
            zf.writestr("manifest.bin", self.binary_manifest)
            zf.writestr("manifest.bin.sig", self.binary_manifest_signature)
            zf.writestr("code.bin", AuthenticatedPage.export(self.code_pages))
            zf.writestr("data.bin", AuthenticatedPage.export(self.data_pages))


def get_device_keys(app_file) -> Struct:
    client = get_client()

    data = Elf(app_file).app_hash()
    apdu = exchange(client, ins=0x10, data=data, cla=0x34)
    assert apdu.status == 0x9000

    struct = Struct("pubkey" / Bytes(65), "encrypted_keys" / Bytes(32), "signature" / Bytes(72), "sig_size" / Int8ul)
    assert len(apdu.data) == struct.sizeof()

    return struct.parse(apdu.data)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("encryption")

    parser = argparse.ArgumentParser(description="Tool to encrypt app ELF binaries.")
    parser.add_argument("--app-file", type=str, help="application path (.elf)")
    parser.add_argument("--output-file", type=str, help="path to the encrypted app (.zip)")
    parser.add_argument("--print-hsm-pubkey", action="store_true", help="print HSM public key")
    parser.add_argument("--show-manifest", type=str, help="display an app manifest in a readable format (.zip)")
    parser.add_argument("--speculos", action="store_true", help="use speculos")

    args = parser.parse_args()

    if args.print_hsm_pubkey:
        print(f"{HSM.get_pubkey_bytes().hex()}")
        sys.exit(0)

    if args.show_manifest:
        zip_file = args.show_manifest
        app = EncryptedApp.from_zip(zip_file)
        manifest = Manifest.from_binary(app.binary_manifest)
        print(manifest)
        sys.exit(1)

    if not args.app_file or not args.output_file:
        logger.error("--app-file and --output-file are required")
        sys.exit(1)

    import_ledgerwallet(args.speculos)
    device_keys = get_device_keys(args.app_file)

    app = EncryptedApp(args.app_file, device_keys)
    app.export_zip(args.output_file)

    # ensure that the generated file is valid
    EncryptedApp.from_zip(args.output_file)
