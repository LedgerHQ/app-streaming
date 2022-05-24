#!/usr/bin/env python3

import hashlib
import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..", "host"))

import stream  # noqa: E402


if __name__ == "__main__":
    parser = stream.get_stream_arg_parser()
    args = parser.parse_args()

    with stream.get_streamer(args) as streamer:
        to_hash = b"a" * 1024

        rsize = len(to_hash).to_bytes(4, "little")
        data = streamer.exchange(rsize)
        assert data == b"ok"

        data = streamer.exchange(to_hash)
        print(f"hexdigest: {data!r}")
        assert data is not None
        assert hashlib.sha256(to_hash).hexdigest() == data.decode()
