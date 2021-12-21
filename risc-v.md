- https://embeddedinn.xyz/articles/tutorial/exploring_virtualization_in_riscv_machines/
- https://varshaaks.wordpress.com/2021/07/19/rv32i-base-integer-instruction-set/
- https://mark.theis.site/riscv/

- https://github.com/bit-hack/riscv-vm
- https://github.com/gamozolabs/fuzz_with_emus
- https://github.com/shady831213/terminus
- https://github.com/d0iasm/rvemu




```
docker pull dockcross/linux-riscv32
docker run --rm dockcross/linux-riscv32:latest > dockcross-linux-riscv32-latest
chmod +x dockcross-linux-riscv32-latest
```

### App VM

1. start app, name, merkle tree
2. wait of requests

Requests:

- request data page (number)
- request code page (number)
- commit data page (number, data)

### App

- APDU input (wait for command, ask for data)
- Button input (left, right, both)

- If not in a command that ask for confirmation, update the menus
- If in a command that ask for confirmation, block any data

New syscalls:

- `wait_for_input` => return when there's input available APDU/button
- `recv_data` (blocking + timeout, buttons interrupt recv, or ignored) / `send_data`
- `recv_button` (blocking + timeout, if APDU received, send error)
- storage!


- https://github.com/LedgerHQ/ledger-nanos-ui/blob/master/src/ui.rs
- https://github.com/LedgerHQ/ledger-nanos-sdk/blob/0d676b552f81d00b96bd9a09f28565086a5efb1f/src/io.rs#L144


## Newlib


```
./configure --host x86
make

cd newlib
CFLAGS='-march=rv32g' ./configure --host riscv32-unknown-linux-gnu --target riscv32-unknown-linux-gnu --enable-multilib --disable-newlib-supplied-syscalls
make
make install
```

- https://interrupt.memfault.com/blog/boostrapping-libc-with-newlib
- https://stackoverflow.com/questions/48160285/rv32e-version-of-the-soft-float-methods-such-as-divdi3-and-mulsi3
- https://stackoverflow.com/questions/50214840/risc-v-startup-code-for-stack-pointer-initalization
