#!/usr/bin/env python3

import os
import random
import sys

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes, serialization

# ensure that protobuf/message_pb2.py will be found
sys.path.append(os.path.dirname(os.path.realpath(__file__)))

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "host"))

# apt install python3-protobuf
from protobuf import message_pb2, swap_pb2  # noqa: E402


import stream  # noqa: E402


class Partner:
    def __init__(self) -> None:
        self.name = b"partner"
        private_value = int.from_bytes([random.randint(0, 255) for _ in range(0, 32)], "big")
        self.private_key = ec.derive_private_key(private_value, ec.SECP256K1(), default_backend())

    def get_pubkey(self) -> bytes:
        format = serialization.PublicFormat.UncompressedPoint
        encoding = serialization.Encoding.X962
        return self.private_key.public_key().public_bytes(encoding=encoding, format=format)

    def ledger_sign(self) -> bytes:
        data = len(self.name).to_bytes(1, "big") + self.name + self.get_pubkey()
        private_value = int.from_bytes(b"\xb1\xed\x47\xef\x58\xf7\x82\xe2\xbc\x4d\x5a\xbe\x70\xef\x66\xd9"
                                       b"\x00\x9c\x29\x57\x96\x70\x17\x05\x44\x70\xe0\xf3\xe1\x0f\x58\x33", "big")
        private_key = ec.derive_private_key(private_value, ec.SECP256K1(), default_backend())
        return private_key.sign(data, ec.ECDSA(hashes.SHA256()))

    def sign_tx(self, tx) -> bytes:
        return self.private_key.sign(tx, ec.ECDSA(hashes.SHA256()))


class Tx:
    def __init__(self, payin, payout, refund, amount_to_wallet, amount_to_provider) -> None:
        tx = swap_pb2.NewTransactionResponse()

        tx.payin_address = payin
        tx.payout_address = payout
        tx.refund_address = refund
        tx.currency_from = "ETH"
        tx.currency_to = "BTC"
        tx.amount_to_wallet = amount_to_wallet
        tx.amount_to_provider = amount_to_provider
        tx.device_transaction_id = "QQQQQQQQQQ"

        self.tx = tx

    def __bytes__(self) -> bytes:
        return self.tx.SerializeToString()


class Swap:
    def init_swap_prepare_request(self) -> bytes:
        init_swap = message_pb2.RequestInitSwap()
        message = message_pb2.Request()
        message.init_swap.CopyFrom(init_swap)
        return message.SerializeToString()

    def init_swap_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        assert response.WhichOneof("response") == "init_swap"
        print(f"device id: {response.init_swap.device_id}")
        self.device_id = response.init_swap.device_id
        return response

    def swap_prepare_request(self) -> bytes:
        payin = "0xd692Cb1346262F584D17B4B470954501f6715a82"
        payout = "bc1qwpgezdcy7g6khsald7cww42lva5g5dmasn6y2z"
        refund = "0xDad77910DbDFdE764fC21FCD4E74D71bBACA6D8D"

        partner = Partner()
        swap = message_pb2.RequestSwap()
        swap.partner.name = partner.name
        swap.partner.pubkey = partner.get_pubkey()
        swap.partner.signature = partner.ledger_sign()

        # ETH 0.00084 (840000000000000 wei)
        swap.fee = b"\x02\xfb\xf9\xbd\x9c\x80\x00"
        # BTC 1 (100000000 sat)
        amount_to_wallet = b"\x05\xf5\xe1\x00"
        # ETH 1.1234 (1123400000000000000 wei)
        amount_to_provider = b"\x0f\x97\x1e\x59\x14\xac\x80\x00"

        tx = Tx(payin, payout, refund, amount_to_wallet, amount_to_provider)
        swap.pb_tx = bytes(tx)
        swap.signature = partner.sign_tx(swap.pb_tx)

        swap.payout_path.extend([0x80000054, 0x80000000, 0x80000000, 1, 0])
        swap.refund_path.extend([0x8000002c, 0x8000003c, 0x80000000, 0, 0])

        message = message_pb2.Request()
        message.swap.CopyFrom(swap)
        return message.SerializeToString()

    def swap_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        if response.WhichOneof("response") == "error":
            print(f"error: {response.error.error_msg}")
            sys.exit(1)
        assert response.WhichOneof("response") == "swap"
        print(f"response: {response.swap}")
        return response

    def version_prepare_request(self):
        version = message_pb2.RequestGetVersion()
        message = message_pb2.Request()
        message.get_version.CopyFrom(version)
        return message.SerializeToString()

    def version_parse_response(self, data):
        response = message_pb2.Response()
        response.ParseFromString(data)
        print(response)
        assert response.WhichOneof("response") == "get_version"
        print(f"version: {response.get_version.version}")
        return response


if __name__ == "__main__":
    parser = stream.get_stream_arg_parser()
    parser.add_argument("--action", default="swap", choices=["swap", "version"])
    args = parser.parse_args()

    with stream.get_streamer(args) as streamer:
        swap = Swap()

        if args.action == "swap":
            data = streamer.exchange(swap.init_swap_prepare_request())
            swap.init_swap_parse_response(data)

            data = streamer.exchange(swap.swap_prepare_request())
            swap.swap_parse_response(data)
        elif args.action == "version":
            data = streamer.exchange(swap.version_prepare_request())
            swap.version_parse_response(data)
