import os
import sys

# ensure that protobuf/message_pb2.py will be found
sys.path.append(os.path.dirname(os.path.realpath(__file__)))

# apt install python3-protobuf
from protobuf import message_pb2  # noqa: E402


class Plugin:
    def __init__(self):
        self.name = "ethereum"
        # 44'/60'/0/0
        self.path = [0x8000002c, 0x8000003c, 0, 0]

    def get_version_prepare_request(self):
        get_version = message_pb2.RequestGetVersion()
        message = message_pb2.Request()
        message.get_version.CopyFrom(get_version)
        assert message.WhichOneof("message_oneof") == "get_version"
        return message.SerializeToString()

    def get_version_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "get_version"
        print(f"version: {response.get_version.version}")
        return response

    def get_pubkey_prepare_request(self):
        get_pubkey = message_pb2.RequestGetPubKey()
        get_pubkey.path.extend(self.path)
        get_pubkey.confirm = False
        get_pubkey.get_chain_code = True
        message = message_pb2.Request()
        message.get_pubkey.CopyFrom(get_pubkey)
        assert message.WhichOneof("message_oneof") == "get_pubkey"
        return message.SerializeToString()

    def get_pubkey_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "get_pubkey"
        return response

    def sign_tx_prepare_request(self):
        sign_tx = message_pb2.RequestSignTx()
        sign_tx.path.extend(self.path)
        sign_tx.raw_tx = bytes.fromhex("f080843b9aca0083015f90946b477781b0e68031109f21887e6b5afeaaeb002b808c5468616e6b732c206d616e21038080")
        sign_tx.chain_id = 1
        message = message_pb2.Request()
        message.sign_tx.CopyFrom(sign_tx)
        assert message.WhichOneof("message_oneof") == "sign_tx"
        return message.SerializeToString()

    def sign_tx_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "sign_tx"
        print(f"signature: {response.sign_tx.signature.hex()}")
        return response

    def sign_msg_prepare_request(self):
        sign_msg = message_pb2.RequestSignMsg()
        sign_msg.path.extend(self.path)
        sign_msg.message = b"Hello world"
        message = message_pb2.Request()
        message.sign_msg.CopyFrom(sign_msg)
        assert message.WhichOneof("message_oneof") == "sign_msg"
        return message.SerializeToString()

    def sign_msg_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "sign_msg"
        print(f"signature: {response.sign_msg.signature.hex()}")
        return response

    def sign_eip712_prepare_request(self):
        sign_eip712 = message_pb2.RequestSignEip712()
        sign_eip712.path.extend(self.path)
        sign_eip712.domain_separator = bytes.fromhex("f2cee375fa42b42143804025fc449deafd50cc031ca257e0b194a650a912090f")
        sign_eip712.message = '{"from": {"name": "Cow", "wallet": "0xCD2a3d9F938E13CD947Ec05AbC7FE734Df8DD826"}, "to": {"name": "Bob", "wallet": "0xbBbBBBBbbBBBbbbBbbBbbbbBBbBbbbbBbBbbBBbB"}, "contents": "Hello, Bob!"}'
        message = message_pb2.Request()
        message.sign_eip712.CopyFrom(sign_eip712)
        assert message.WhichOneof("message_oneof") == "sign_eip712"
        return message.SerializeToString()

    def sign_eip712_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("message_oneof") == "sign_eip712"
        print(f"signature: {response.sign_eip712.signature.hex()}")
        return response
