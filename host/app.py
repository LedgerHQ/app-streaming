import argparse
import logging
import sys

from construct import Bytes, Int8ul, Struct
from Crypto.Cipher import AES
from typing import cast, List, Optional, Type
from zipfile import ZipFile

from comm import get_client, CommClient
from elf import Segment
from manifest import Manifest


class App:
    def __init__(self, manifest: bytes, hsm_signature: bytes, code_pages: bytes, data_pages: bytes,
                 device_signature: Optional[bytes] = None, device_pubkey: Optional[bytes] = None,
                 ) -> None:
        self.code_pages = App._pages_to_list(code_pages)
        self.data_pages = App._pages_to_list(data_pages)

        if device_pubkey:
            self.device_pubkey = device_pubkey

        self.manifest = manifest
        self.manifest_hsm_signature = hsm_signature

    @staticmethod
    def _pages_to_list(data: bytes) -> List[bytes]:
        pages = []

        assert len(data) % Segment.PAGE_SIZE == 0

        for offset in range(0, len(data), Segment.PAGE_SIZE):
            page_data = data[offset:offset+Segment.PAGE_SIZE]
            pages.append(page_data)

        return pages

    @classmethod
    def from_zip(cls: Type[object], zip_path: str) -> "App":
        app = cast(App, cls.__new__(cls))
        with ZipFile(zip_path, "r") as zf:
            app.manifest = zf.read("manifest.bin")
            app.manifest_hsm_signature = zf.read("manifest.hsm.sig")
            app.code_pages = App._pages_to_list(zf.read("code.bin"))
            app.data_pages = App._pages_to_list(zf.read("data.bin"))

        return app

    def export_zip(self, zip_path: str) -> None:
        with ZipFile(zip_path, "w") as zf:
            zf.writestr("manifest.bin", self.manifest)
            zf.writestr("manifest.hsm.sig", self.manifest_hsm_signature)
            zf.writestr("code.bin", b"".join(self.code_pages))
            zf.writestr("data.bin", b"".join(self.data_pages))

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("app")

    parser = argparse.ArgumentParser(description="Tool to sign app using a Ledger device.")
    parser.add_argument("--app-path", type=str, help="path to the app (.zip)")
    parser.add_argument("--show-manifest", action="store_true",
                        help="display an app manifest in a readable format (.zip)")
    parser.add_argument("--speculos", action="store_true", help="use speculos")
    parser.add_argument("--transport", default="usb", choices=["ble", "usb"])

    args = parser.parse_args()

    if not args.app_path:
        logger.error("--app-path is required")
        sys.exit(1)

    app = App.from_zip(args.app_path)

    if args.show_manifest:
        manifest = Manifest(app.manifest)
        print(manifest)
        sys.exit(1)
