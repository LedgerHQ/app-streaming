import socket


class Server:
    """Basic TCP server which except a single client and a single exchange."""

    def __init__(self, port=1111):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.s.bind(("0.0.0.0", port))  # lgtm [py/bind-socket-all-network-interfaces]
        self.s.listen(1)

        self.c = self.wait_for_client()

    def wait_for_client(self):
        c, _ = self.s.accept()
        return c

    @staticmethod
    def _recvall(s, size):
        data = b""
        while size > 0:
            chunk = s.recv(size)
            if chunk is None:
                raise Exception("Server: Connection closed with client")
            data += chunk
            size -= len(chunk)
        return data

    def recv_request(self):
        """Receive a request from a client."""

        data = Server._recvall(self.c, 4)
        size = int.from_bytes(data, "little")
        data = Server._recvall(self.c, size)

        return data

    def send_response(self, data):
        """Forwared a response from the device to the client."""

        size = len(data).to_bytes(4, "little")
        self.c.sendall(size)
        self.c.sendall(data)
        self.c.close()

        self.s.close()
