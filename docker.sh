#!/bin/bash

# Run the docker image with the RISC-V toolchain.

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ ! -f ${SCRIPT_DIR}/app/dockcross-linux-riscv32-latest ]; then
    docker run --rm dockcross/linux-riscv32:latest > ${SCRIPT_DIR}/app/dockcross-linux-riscv32-latest
    chmod 0755 ${SCRIPT_DIR}/app/dockcross-linux-riscv32-latest
fi

docker run -u $(id -u):$(id -g) --env=SUDO_PS1=1 --env=SUDO_USER=1 --env=PS1='[\[\033[01;33m\]user@\h\[\033[00m\]:\[\033[01;33m\]\w\[\033[00m\]] \[\033[01;91m\]\$\[\033[01;35m\]\[\033[00m\] ' -w /app -v ${SCRIPT_DIR}/app/:/app/ --rm -it riscv bash
