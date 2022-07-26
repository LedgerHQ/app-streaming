#!/usr/bin/env python3

import argparse
import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "target/debug/"))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../host/"))

import libstreaming as streaming
import comm as host_comm

if __name__ == "__main__":
    path = "/tmp/app.zip"
    with host_comm.get_client(use_speculos=True) as comm:
        if False:
            data = streaming.get_pubkey(path, comm)
            print(f"pubkey: {data.hex()}")
            streaming.device_sign_app(path, comm)
        else:
            stream = streaming.Stream(path, comm)
            stream.exchange(b"lol")
