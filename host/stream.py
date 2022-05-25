#!/usr/bin/env python3

import sys

from argparse import ArgumentParser
from typing import Union

from stream_device import DeviceStreamer
from stream_native import NativeStreamer


def get_stream_arg_parser() -> ArgumentParser:
    parser = ArgumentParser(description="RISC-V vm companion.")
    parser.add_argument("--app", type=str, required=True, help="encrypted application path (.zip)")
    parser.add_argument("--native", action="store_true", help="run native apps")
    parser.add_argument("--speculos", action="store_true", help="use speculos")
    parser.add_argument("--transport", default="usb", choices=["ble", "usb"])
    parser.add_argument("--verbose", action="store_true", help="")
    return parser


def get_streamer(args) -> Union[DeviceStreamer, NativeStreamer]:
    """
    A metaclass would be better than a function but I failed at making it work
    with typing.
    """
    if args.native:
        return NativeStreamer(args)
    else:
        return DeviceStreamer(args)


if __name__ == "__main__":
    parser = get_stream_arg_parser()
    args = parser.parse_args()

    with get_streamer(args) as streamer:
        while True:
            # read hex line from stdin
            print("> ", end="")
            sys.stdout.flush()
            request = bytes.fromhex(sys.stdin.readline().strip())

            response = streamer.exchange(request)
            if response is None:
                break

            print("<", response.hex())
