#!/bin/bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p "${SCRIPT_DIR}/risc-v/" "${SCRIPT_DIR}/x64/"

# retrieve /usr/local/lib/libcrypto.a from the native container
docker run --rm --entrypoint tar -i native -cf - /usr/local/lib/libcrypto.a \
    | tar -C "${SCRIPT_DIR}/x64/" --strip-components=3 -xf -

# retrieve /usr/local/riscv32-unknown-linux-gnu/lib/libc.a from the risc-v container
docker run --rm --entrypoint tar -i riscv -cf - /usr/local/riscv32-unknown-linux-gnu/lib/libc.a \
    | tar -C "${SCRIPT_DIR}/risc-v/" --strip-components=4 -xf -

find "${SCRIPT_DIR}" -type f -name 'lib*.a' -ls
