import hashlib
import logging

logger = logging.getLogger("plugin")

class Plugin:
    def __init__(self):
        logger.debug("plugin loaded")
        self.state = -1
        self.data = b"a" * 1024

    def recv(self, data):
        print(f"hexdigest: {data}")

        assert hashlib.sha256(self.data).hexdigest() == data.decode()

        # exit directly once the digest is received, for benchmarks
        import sys
        sys.exit(0)

    def send(self):
        self.state += 1

        if self.state == 0:
            return len(self.data).to_bytes(4, "little")
        elif self.state == 1:
            return self.data
        else:
            assert False
