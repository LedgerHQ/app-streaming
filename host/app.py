import argparse
import logging
import sys

from construct import Bytes, Int8ul, Struct
from Crypto.Cipher import AES
from typing import List, Optional, Type
from zipfile import ZipFile

from comm import get_client, import_ledgerwallet
from elf import Segment
from manifest import Manifest


class App:
    def __init__(self, manifest: bytes, hsm_signature: bytes, code_pages: bytes, data_pages: bytes, device_signature: Optional[bytes] = None, code_macs: Optional[bytes] = None, data_macs: Optional[bytes] = None) -> None:
        self.code_pages = App._pages_to_list(code_pages)
        self.data_pages = App._pages_to_list(data_pages)

        if code_macs:
            self.code_macs = App._macs_to_list(code_macs)
        if data_macs:
            self.data_macs = App._macs_to_list(data_macs)

        self.manifest = manifest
        self.manifest_hsm_signature = hsm_signature
        self.manifest_device_signature = device_signature

    @staticmethod
    def _pages_to_list(data: bytes) -> List[bytes]:
        pages = []

        assert len(data) % Segment.PAGE_SIZE == 0

        for offset in range(0, len(data), Segment.PAGE_SIZE):
            page_data = data[offset:offset+Segment.PAGE_SIZE]
            pages.append(page_data)

        return pages

    @staticmethod
    def _macs_to_list(data: bytes) -> List[bytes]:
        macs = []

        assert len(data) % 32 == 0

        for offset in range(0, len(data), 32):
            mac = data[offset:offset+32]
            macs.append(mac)

        return macs

    @classmethod
    def from_zip(cls: Type[object], zip_path: str):
        app = cls.__new__(cls)
        with ZipFile(zip_path, "r") as zf:
            app.manifest = zf.read("manifest.bin")
            app.manifest_hsm_signature = zf.read("manifest.hsm.sig")
            app.code_pages = App._pages_to_list(zf.read("code.bin"))
            app.data_pages = App._pages_to_list(zf.read("data.bin"))

            if "device/manifest.device.sig" in zf.namelist():
                app.manifest_device_signature = zf.read("device/manifest.device.sig")
                app.code_macs = App._macs_to_list(zf.read("device/code.mac.bin"))
                app.data_macs = App._macs_to_list(zf.read("device/data.mac.bin"))
            else:
                app.manifest_device_signature = None
                app.code_macs = None
                app.data_macs = None

        return app

    def export_zip(self, zip_path: str) -> None:
        with ZipFile(zip_path, "w") as zf:
            zf.writestr("manifest.bin", self.manifest)
            zf.writestr("manifest.hsm.sig", self.manifest_hsm_signature)
            zf.writestr("code.bin", b"".join(self.code_pages))
            zf.writestr("data.bin", b"".join(self.data_pages))

            if self.manifest_device_signature:
                zf.writestr("device/manifest.device.sig", self.manifest_device_signature)
                zf.writestr("device/code.mac.bin", b"".join(self.code_macs))
                zf.writestr("device/data.mac.bin", b"".join(self.data_macs))


def decrypt_hmac(enc_macs, aes):
    result = []
    for enc_mac in enc_macs:
        mac = aes.decrypt(enc_mac)
        result.append(mac)
    return result


def device_sign_app(app: App, transport: str) -> None:
    client = get_client(transport)

    signature = app.manifest_hsm_signature
    data = app.manifest + signature.ljust(72, b"\x00") + len(signature).to_bytes(1, "little")

    apdu = client.exchange(ins=0x11, data=data, cla=0x34)
    assert apdu.status == 0x6801

    code_macs: List[bytes] = []
    data_macs: List[bytes] = []
    enc_macs = [code_macs, data_macs]
    for pages in [app.code_pages, app.data_pages]:
        macs = enc_macs.pop(0)
        for page in pages:
            assert apdu.status == 0x6801

            apdu = client.exchange(0x01, data=page[1:], p2=page[0], cla=0x34)
            assert apdu.status == 0x6802
            print(hex(apdu.status), apdu.data.hex())
            mac = apdu.data
            macs.append(mac)

            apdu = client.exchange(0x01, data=b"")

    assert apdu.status == 0x9000

    struct = Struct("aes_key" / Bytes(32), "signature" / Bytes(72), "sig_size" / Int8ul)
    assert len(apdu.data) == struct.sizeof()
    s = struct.parse(apdu.data)

    iv = b"\x00" * 16
    aes = AES.new(s.aes_key, AES.MODE_CBC, iv)
    app.code_macs = decrypt_hmac(code_macs, aes)
    app.data_macs = decrypt_hmac(data_macs, aes)
    app.manifest_device_signature = s.signature[:s.sig_size]


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s.%(msecs)03d:%(name)s: %(message)s', datefmt='%H:%M:%S')
    logger = logging.getLogger("app")

    parser = argparse.ArgumentParser(description="Tool to sign app using a Ledger device.")
    parser.add_argument("--app-path", type=str, help="path to the app (.zip)")
    parser.add_argument("--show-manifest", action="store_true", help="display an app manifest in a readable format (.zip)")
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

    import_ledgerwallet(args.speculos)
    device_sign_app(app, args.transport)
    app.export_zip(args.app_path)

    # ensure that the generated file is valid
    app = App.from_zip(args.app_path)
