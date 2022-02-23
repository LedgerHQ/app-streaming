import hashlib


class Plugin:
    def __init__(self):
        self.name = "sha256"
        self.data = b"a" * 1024

    def hash1_prepare_request(self):
        return len(self.data).to_bytes(4, "little")

    def hash1_parse_response(self, data):
        print(data)
        assert data == b"ok"

    def hash2_prepare_request(self):
        return self.data

    def hash2_parse_response(self, data):
        print(f"hexdigest: {data}")

        assert hashlib.sha256(self.data).hexdigest() == data.decode()

        return data
