#!/bin/bash

set -e

docker run -w /app -v $(pwd)/app/:/app/ --rm -it dockcross/linux-riscv32 bash
