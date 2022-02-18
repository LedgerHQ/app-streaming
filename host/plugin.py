class Plugin:
    """Default plugin"""

    def __init__(self):
        pass

    def recv(self, data):
        """This function is called when the app sends data to the client."""

        print(f"received {data}")

    def send(self):
        """This function is called when the app asks data from the client."""

        return b"a" * 1000
