import hashlib
import hmac

from Crypto.Cipher import AES


class Encryption:
    def __init__(self):
        self.hmac_key = b"k" * 32
        self.encryption_key = b"K" * 32

    def _hmac(self, addr, data, iv):
        msg = data + addr.to_bytes(4, "little") + iv.to_bytes(4, "little")
        return hmac.new(self.hmac_key, msg, digestmod=hashlib.sha256).digest()

    def _encrypt(self, data, addr, iv):
        iv = addr.to_bytes(4, "little") + int(iv).to_bytes(4, "little")
        iv = iv.ljust(16, b"\x00")

        aes = AES.new(self.encryption_key, AES.MODE_CBC, iv)
        return aes.encrypt(data)

    def encrypt_segment(self, segment, page_size, iv):
        pages = {}

        addr = segment.start
        for offset in range(0, segment.size, page_size):
            page = segment.data[offset:offset+page_size]
            assert len(page) == page_size

            encrypted_page = self._encrypt(page, addr, iv)
            assert len(encrypted_page) == page_size

            assert addr not in pages
            pages[addr] = (encrypted_page, self._hmac(addr, encrypted_page, iv))

            addr += page_size

        if False:
            for addr, (_, digest) in pages.items():
                print(f"{addr:#x} {digest.hex()}")

        return pages
