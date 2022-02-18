import logging

logger = logging.getLogger("plugin")

class Plugin:
    def __init__(self):
        logger.debug("plugin loaded")
        self.state = -1

    def recv(self, data):
        print(f"hexdigest: {data}")

    def send(self):
        self.state += 1

        data = b"a" * 1000

        if self.state == 0:
            return len(data).to_bytes(4, "little")
        elif self.state == 1:
            return data
        else:
            assert False
