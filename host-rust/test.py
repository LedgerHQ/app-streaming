#!/usr/bin/env python3

import argparse
import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "target/debug/"))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../host/"))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "../app/app-swap/"))

import libstreaming as streaming
import comm as host_comm
from swap import Swap

if __name__ == "__main__":
    path = "/tmp/app.zip"
    with host_comm.get_client(use_speculos=True) as comm:
        if False:
            data = streaming.get_pubkey(path, comm)
            print(f"pubkey: {data.hex()}")
            streaming.device_sign_app(path, comm)
        else:
            streamer = streaming.Stream(path, comm)

            swap = Swap()
            if False:
                data = streamer.exchange(swap.version_prepare_request())
                print(swap.version_parse_response(data))
            else:
                data = streamer.exchange(swap.init_swap_prepare_request())
                print(swap.init_swap_parse_response(data))

                data = streamer.exchange(swap.swap_prepare_request())
                print(swap.swap_parse_response(data))
