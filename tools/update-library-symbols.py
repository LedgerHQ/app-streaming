#!/usr/bin/env python3

"""
This script renames (converts to uppercase) some symbols in a library.
"""

import lief
import os
import subprocess
import sys
import tempfile

SYMBOLS = [
    "blake2b_final",
    "blake2b_init",
    "blake2b_init_key",
    "blake2b_init_param",
    "blake2b_update",
    "cx_blake2b",
    "cx_blake2b_final",
    "cx_blake2b_get_output_size",
    "cx_blake2b_update",
    "cx_ecfp_decode_sig_der",
    "cx_ecfp_encode_sig_der",
    "cx_hash_final",
    "cx_hash_get_info",
    "cx_hash_get_size",
    "cx_hash_init",
    "cx_hash_init_ex",
    "cx_hash_sha512",
    "cx_hash_update",
    "cx_ripemd160_final",
    "cx_ripemd160_update",
    "cx_sha256_final",
    "cx_sha256_update",
    "cx_sha3_block",
    "cx_sha3_final",
    "cx_sha3_get_output_size",
    "cx_sha3_update",
    "cx_sha512_final",
    "cx_sha512_update",
    "cx_swap_buffer32",
    "cx_swap_buffer64",
    "cx_swap_uint32",
    "cx_swap_uint64",
]


def update_symbols(obj_path: str):
    """https://stackoverflow.com/a/65453266"""

    lib = lief.parse(obj_path)
    for x in lib.symbols:
        if x.name in SYMBOLS:
            x.name = x.name.upper()

    lib.write(obj_path)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <lib.a>")
        sys.exit(1)

    ar_path = sys.argv[1]

    with tempfile.TemporaryDirectory() as tmp_dir:
        subprocess.check_call(f"rm -rf {tmp_dir}", shell=True)
        subprocess.check_call(f"mkdir {tmp_dir}", shell=True)
        subprocess.check_call(f"ar --output={tmp_dir} x {ar_path}", shell=True)

        for obj_path in os.listdir(tmp_dir):
            update_symbols(os.path.join(tmp_dir, obj_path))

        subprocess.check_call(f"ar r {ar_path} {tmp_dir}/*.o", shell=True)
