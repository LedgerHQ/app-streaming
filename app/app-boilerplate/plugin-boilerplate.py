import os
import sys

# ensure that protobuf/message_pb2.py will be found
sys.path.append(os.path.dirname(os.path.realpath(__file__)))

# apt install python3-protobuf
from protobuf import message_pb2  # noqa: E402


class Plugin:
    def __init__(self):
        self.name = "boilerplate"

    def get_version_prepare_request(self):
        get_version = message_pb2.MessageGetVersion()
        message = message_pb2.Message()
        message.get_version.CopyFrom(get_version)
        assert message.WhichOneof("message_oneof") == "get_version"
        return message.SerializeToString()

    def get_version_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "get_version"
        print(f"version: {response.get_version.version}")
        return response

    def sign_tx_prepare_request(self):
        sign_tx = message_pb2.MessageSignTx()
        sign_tx.tx = b"ThisIsAComplicatedTransaction"
        message = message_pb2.Message()
        message.sign_tx.CopyFrom(sign_tx)
        assert message.WhichOneof("message_oneof") == "sign_tx"
        return message.SerializeToString()

    def sign_tx_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "sign_tx"
        print(f"signature: {response.sign_tx.signature.hex()}")
        return response
