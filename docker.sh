#!/bin/bash

# Helper to run various docker images.

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

docker_run() {
    image="${1}"

    shift

    docker run -u "$(id -u):$(id -g)" \
       --env=SUDO_PS1=1 --env=SUDO_USER=1 --env=PS1='[\[\033[01;33m\]user@\h\[\033[00m\]:\[\033[01;33m\]\w\[\033[00m\]] \[\033[01;91m\]\$\[\033[01;35m\]\[\033[00m\] ' \
       -w /app \
       -v "${SCRIPT_DIR}/app/":/app/ \
       -v "${SCRIPT_DIR}/host/":/host/:ro \
       "$@" \
       --rm -it "${image}" \
       bash

}

if [ $# -eq 0 ]; then
    image='riscv'
elif [ $# -eq 1 ]; then
    image="${1}"
else
    echo "Usage: ${0} <riscv|native|rust>"
    exit 1
fi

case ${image} in
    riscv)
        if [ ! -f "${SCRIPT_DIR}/app/dockcross-linux-riscv32-latest" ]; then
            docker run --rm dockcross/linux-riscv32:latest > "${SCRIPT_DIR}/app/dockcross-linux-riscv32-latest"
            chmod 0755 "${SCRIPT_DIR}/app/dockcross-linux-riscv32-latest"
        fi
        docker_run riscv
        ;;

    native)
        docker_run native \
                   --env SPECULOS_DIR=/speculos/ \
                   -v "${HOME}/code/speculos/":/speculos/:ro
        ;;

    rust)
        docker_run rust
        ;;

    *)
        echo "Invalid image name"
        exit 1
        ;;
esac
