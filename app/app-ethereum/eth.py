#!/usr/bin/env python3

import json
import os
import sys

# ensure that protobuf/message_pb2.py will be found
sys.path.append(os.path.dirname(os.path.realpath(__file__)))

# apt install python3-protobuf
from protobuf import message_pb2  # noqa: E402

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "host"))

import stream  # noqa: E402


def tohex(n):
    result = f"{n:x}"
    if len(result) % 2 != 0:
        result = "0" + result
    return f"0x{result}"


def convert_int(item):
    """
    XXX: this is plain wrong and the JSON schema should be used.
    """
    if isinstance(item, dict):
        for k, v in item.items():
            item[k] = convert_int(v)
        return item
    elif isinstance(item, list):
        return [convert_int(v) for v in item]
    elif isinstance(item, str):
        if not item.isnumeric():
            return item
        return tohex(int(item, 10))
    elif isinstance(item, int):
        return tohex(item)
    else:
        return item


def eip712_encode_json(message):
    v = json.loads(message)
    v = convert_int(v)
    return json.dumps(v, separators=(',', ':'))


class Eth:
    def __init__(self):
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
        raw_tx = "f080843b9aca0083015f90946b477781b0e68031109f21887e6b5afeaaeb002b808c5468616e6b732c206d616e21038080"
        sign_tx = message_pb2.RequestSignTx()
        sign_tx.path.extend(self.path)
        sign_tx.raw_tx = bytes.fromhex(raw_tx)
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
        if False:
            sign_eip712.domain_separator = bytes.fromhex("f2cee375fa42b42143804025fc449deafd50cc031ca257e0b194a650a912090f")  # noqa: E501
            sign_eip712.message = '{"from": {"name": "Cow", "wallet": "0xCD2a3d9F938E13CD947Ec05AbC7FE734Df8DD826"}, "to": {"name": "Bob", "wallet": "0xbBbBBBBbbBBBbbbBbbBbbbbBBbBbbbbBbBbbBBbB"}, "contents": "Hello, Bob!"}'  # noqa: E501
        else:
            sign_eip712.domain_separator = bytes.fromhex("72982d92449bfb3d338412ce4738761aff47fb975ceb17a1bc3712ec716a5a68")  # noqa: E501
            message = '{"exchange": "0x7f268357a8c2552623316e2562d90e642bb538e5", "maker": "0x112f0732e59e7600768dfc35ba744b89f2356cd8", "taker": "0x0000000000000000000000000000000000000000", "makerRelayerFee": "1250", "takerRelayerFee": "0", "makerProtocolFee": "0", "takerProtocolFee": "0", "feeRecipient": "0x5b3256965e7c3cf26e11fcaf296dfc8807c01073", "feeMethod": 1, "side": 1, "saleKind": 0, "target": "0xbaf2127b49fc93cbca6269fade0f7f31df4c88a7", "howToCall": 1, "calldata": "0x96809f90000000000000000000000000112f0732e59e7600768dfc35ba744b89f2356cd80000000000000000000000000000000000000000000000000000000000000000000000000000000000000000495f947276749ce646f68ac8c248420045cb7b5ebdf2657ffc1fadfd73cf0a8cde95d50b62d3df8c0000000000000700000000320000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e00000000000000000000000000000000000000000000000000000000000000000", "replacementPattern": "0x000000000000000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", "staticTarget": "0x0000000000000000000000000000000000000000", "staticExtradata": "0x", "paymentToken": "0x0000000000000000000000000000000000000000", "basePrice": "2000000000000000000", "extra": "0", "listingTime": "1645484541", "expirationTime": "1646089435", "salt": "21014297276898013168171430966355369260039074692095359200549020767078729356431", "nonce": 0}'  # noqa: E501
            sign_eip712.message = eip712_encode_json(message)
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


if __name__ == "__main__":
    actions = ["get_version", "get_pubkey", "sign_tx", "sign_msg", "sign_eip712"]

    parser = stream.get_stream_arg_parser()
    parser.add_argument("--action", default="get_version", choices=actions)
    args = parser.parse_args()

    with stream.Streamer(args) as streamer:
        eth = Eth()

        method_name = f"{args.action}_prepare_request"
        prepare_request = getattr(eth, method_name)

        method_name = f"{args.action}_parse_response"
        parse_response = getattr(eth, method_name)

        data = streamer.exchange(prepare_request())
        parse_response(data)
