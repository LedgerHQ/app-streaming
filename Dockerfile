# RISC-V toolchain with newlib

FROM dockcross/linux-riscv32:latest

RUN apt-get -yq update && \
    apt-get -yq install less python3-protobuf protobuf-compiler

RUN git clone --depth 1 git://cygwin.com/git/newlib-cygwin.git /tmp/newlib-cygwin/ && \
    mkdir /tmp/newlib-cygwin/newlib/build && \
    cd /tmp/newlib-cygwin/newlib/build && \
    CFLAGS='-march=rv32g' ../configure --host riscv32-unknown-linux-gnu --target riscv32-unknown-linux-gnu --enable-multilib --disable-newlib-supplied-syscalls --enable-newlib-nano-malloc --enable-lite-exit --disable-newlib-io-float && \
    make && \
    make install && \
    cd / && \
    rm -rf /tmp/newlib-cygwin/

ENTRYPOINT /bin/bash
