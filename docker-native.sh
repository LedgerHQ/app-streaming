#!/bin/bash

# Run the docker image with the x86 toolchain.

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

docker run --network host \
    -u $(id -u):$(id -g) \
    --env=SUDO_PS1=1 --env=SUDO_USER=1 --env=PS1='[\[\033[01;33m\]user@\h\[\033[00m\]:\[\033[01;33m\]\w\[\033[00m\]] \[\033[01;91m\]\$\[\033[01;35m\]\[\033[00m\] ' \
    --env SPECULOS_DIR=/speculos/ \
    -w /app \
    -v ${SCRIPT_DIR}/app/:/app/ \
    -v ${SCRIPT_DIR}/host/:/host/:ro \
    -v ${HOME}/code/speculos/:/speculos/:ro \
    --rm -it native \
    bash
