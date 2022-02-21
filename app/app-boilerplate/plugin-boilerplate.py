import logging
import os
import sys

# ensure that protobuf/message_pb2.py will be found
sys.path.append(os.path.dirname(os.path.realpath(__file__)))

# apt install python3-protobuf
from protobuf import message_pb2  # noqa: E402

logger = logging.getLogger("plugin")


class Plugin:
    def __init__(self):
        logger.debug("plugin loaded")
        self.state = 0

    def recv(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)

        if self.state == 0:
            assert response.WhichOneof("message_oneof") == "get_version"
            print(f"version: {response.get_version.version}")
        elif self.state == 1:
            assert response.WhichOneof("message_oneof") == "sign_tx"
            print(f"version: {response.sign_tx.signature.hex()}")

        self.state += 1

    def send(self):
        message = message_pb2.Message()
        if self.state == 0:
            get_version = message_pb2.MessageGetVersion()
            message.get_version.CopyFrom(get_version)
            assert message.WhichOneof("message_oneof") == "get_version"
        elif self.state == 1:
            sign_tx = message_pb2.MessageSignTx()
            sign_tx.tx = b"ThisIsAComplicatedTransaction"
            message.sign_tx.CopyFrom(sign_tx)
            assert message.WhichOneof("message_oneof") == "sign_tx"
        else:
            assert False

        return message.SerializeToString()
