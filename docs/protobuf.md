# protobuf

- https://github.com/protocolbuffers/protobuf/blob/master/src/README.md

```console
git clone --depth 1 https://github.com/protocolbuffers/protobuf
git submodule update --init --recursive
./autogen.sh

CC=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-gcc CXX=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-g++ ./configure --target=riscv32 --host=x86_64-pc-linux-gnu --with-protoc=protoc
```

# protoc x86

```
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protoc-3.19.4-linux-x86_64.zip
mkdir protoc && unzip -d protoc protoc-3.19.4-linux-x86_64.zip
```

# protobuf-c

```
apt-get install libprotoc-dev
git clone --depth=1 https://github.com/protobuf-c/protobuf-c
./autogen.sh

PROTOC=/app/app-boilerplate/protoc/bin/protoc CPPFLAGS=-I/app/app-boilerplate/protobuf/src/ CFLAGS='-DNDEBUG -march=rv32g -mno-div -mno-fdiv -mstrict-align -Os' CC=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-gcc CXX=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-g++ ./configure --target=riscv32 --host=x86_64-pc-linux-gnu --disable-protoc
make
```

Feedbacks:

- `NDEBUG` to disable assert (otherwise `__assert_fail` isn't found during linking of the RISC-V app).
- `PROTOC` might not be required thanks to `--disable-protoc`
