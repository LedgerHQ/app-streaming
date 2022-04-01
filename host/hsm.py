import argparse
import hashlib
import logging
import sys

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.ec import EllipticCurvePrivateKey, EllipticCurvePublicKey
from cryptography.hazmat.primitives import hashes, serialization
from typing import List

from app import App
from elf import Elf, Segment
from manifest import Manifest
from merkletree import Entry, MerkleTree


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
    def sign_app(data: bytes) -> bytes:
        private_key = HSM._get_private_key()
        signature = private_key.sign(data, ec.ECDSA(hashes.SHA256()))
        return signature


class HsmApp:
    def __init__(self, path: str) -> None:
        elf = Elf(path)

        self.name = elf.app_infos["name"]
        self.version = elf.app_infos["version"]

        self.entrypoint = elf.entrypoint

        self.code_start, self.code_end = elf.get_section_range("code")
        self.data_start, self.data_end = elf.get_section_range("data")

        self.code = elf.get_segment("code").data
        self.data = elf.get_segment("data").data

        self.code_pages = HsmApp.data_to_pages(self.code)
        self.data_pages = HsmApp.data_to_pages(self.data)

        self.merkletree = HsmApp.compute_merkle_tree(self.data_start, self.data_end)

    def compute_app_hash(self) -> bytes:
        code_size = self.code_end - self.code_start
        data_size = self.data_end - self.data_start

        m = hashlib.sha256()
        m.update(code_size.to_bytes(4, "little"))
        m.update(data_size.to_bytes(4, "little"))
        m.update(self.code)
        m.update(self.data)
        return m.digest()

    @staticmethod
    def compute_merkle_tree(data_start: int, data_end: int) -> MerkleTree:
        merkletree = MerkleTree()
        iv = 0

        for addr in range(data_start, data_end, Segment.PAGE_SIZE):
            merkletree.insert(Entry.from_values(addr, iv))

        return merkletree

    @staticmethod
    def data_to_pages(data: bytes) -> List[bytes]:
        pages = []

        assert len(data) % Segment.PAGE_SIZE == 0

        for offset in range(0, len(data), Segment.PAGE_SIZE):
            pages.append(data[offset:offset+Segment.PAGE_SIZE])

        return pages

    def get_manifest(self) -> Manifest:
        stack_end = 0x80000000
        stack_start = stack_end - Elf.STACK_SIZE

        data = Manifest.MANIFEST_STRUCT.build(dict({
            "name": self.name,
            "version": self.version,
            "app_hash": self.compute_app_hash(),
            "entrypoint": self.entrypoint,
            "bss": self.data_end,
            "code_start": self.code_start,
            "code_end": self.code_end,
            "stack_start": stack_start,
            "stack_end": stack_end,
            "data_start": self.data_start,
            "data_end": self.data_end + Elf.HEAP_SIZE,
            "mt_root_hash": self.merkletree.root_hash(),
            "mt_size": len(self.merkletree.entries),
            "mt_last_entry": self.merkletree.entries[-1],
        }))

        return Manifest.from_binary(data)

    def get_manifest_signature(self) -> bytes:
        manifest = self.get_manifest()
        return HSM.sign_app(manifest.export_binary())


def hsm_sign_app(elf_path: str) -> App:
    hsm_app = HsmApp(elf_path)
    hsm_signature = hsm_app.get_manifest_signature()

    manifest = hsm_app.get_manifest().export_binary()
    return App(manifest, hsm_signature, hsm_app.code, hsm_app.data)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("hsm")

    parser = argparse.ArgumentParser(description="Tool to sign ELF binaries with the HSM.")
    parser.add_argument("--elf-path", type=str, help="application path (.elf)")
    parser.add_argument("--app-path", type=str, help="path to the signed app (.zip)")
    parser.add_argument("--print-hsm-pubkey", action="store_true", help="print HSM public key")

    args = parser.parse_args()

    if args.print_hsm_pubkey:
        print(f"{HSM.get_pubkey_bytes().hex()}")
        sys.exit(0)

    if not args.elf_path or not args.app_path:
        logger.error("--elf-path and --app-path are required")
        sys.exit(1)

    app = hsm_sign_app(args.elf_path)
    app.export_zip(args.app_path)

    # ensure that the generated file is valid
    app = App.from_zip(args.app_path)
