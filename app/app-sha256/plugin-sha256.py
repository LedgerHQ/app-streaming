import logging

logger = logging.getLogger("plugin")

class Plugin:
    def __init__(self):
        logger.debug("plugin loaded")
        self.state = -1

    def recv(self, data):
        print(f"hexdigest: {data}")

        # exit directly once the digest is received, for benchmarks
        import sys
        sys.exit(0)

    def send(self):
        self.state += 1

        data = b"a" * 1024

        if self.state == 0:
            return len(data).to_bytes(4, "little")
        elif self.state == 1:
            return data
        else:
            assert False
