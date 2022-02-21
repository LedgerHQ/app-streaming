# RISC-V toolchain with newlib

FROM dockcross/linux-riscv32:latest

RUN git clone --depth 1 git://cygwin.com/git/newlib-cygwin.git /tmp/newlib-cygwin/ && \
    mkdir /tmp/newlib-cygwin/newlib/build && \
    cd /tmp/newlib-cygwin/newlib/build && \
    CFLAGS='-march=rv32g' ../configure --host riscv32-unknown-linux-gnu --target riscv32-unknown-linux-gnu --enable-multilib --disable-newlib-supplied-syscalls --enable-newlib-nano-malloc --enable-lite-exit && \
    make && \
    make install && \
    cd / && \
    rm -rf /tmp/newlib-cygwin/

RUN wget --quiet https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protoc-3.19.4-linux-x86_64.zip && \
    wget --quiet https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.19.4.zip && \
    wget --quiet https://github.com/protobuf-c/protobuf-c/archive/refs/tags/v1.4.0.zip && \
    unzip -d /usr/local/ protoc-3.19.4-linux-x86_64.zip bin/protoc && \
    unzip v3.19.4.zip && \
    unzip v1.4.0.zip && \
    cd protobuf-c-1.4.0/ && \
    ./autogen.sh && \
    CPPFLAGS="-I$(pwd)/../protobuf-3.19.4/src/" CFLAGS='-DNDEBUG -march=rv32g -mno-div -mno-fdiv -mstrict-align -Os' CC=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-gcc CXX=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-g++ ./configure --target=riscv32 --host=x86_64-pc-linux-gnu --disable-protoc && \
    make && \
    make install && \
    cd .. && \
    rm -rf protoc-3.19.4-linux-x86_64.zip v3.19.4.zip v1.4.0.zip protobuf-3.19.4 protobuf-c-1.4.0

ENTRYPOINT /bin/bash
