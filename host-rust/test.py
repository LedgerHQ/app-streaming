#!/usr/bin/env python3

import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "target/debug/"))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../host/"))

import libstreaming as streaming
import comm as host_comm

class Comm:
    def exchange_(self, data: bytes) -> bytes:
        print(data)
        print(f"{data.hex()}")
        return data


if __name__ == "__main__":
    comm = Comm()
    with host_comm.get_client(use_speculos=True) as comm:
        data = streaming.get_pubkey("blah", comm)
        print(data)
