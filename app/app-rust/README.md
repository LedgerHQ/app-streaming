## Build RISC-V app

```console
cargo build --release
```

```console
$ file target/riscv32i-unknown-none-elf/release/demo
target/riscv32i-unknown-none-elf/release/demo: ELF 32-bit LSB executable, UCB RISC-V, version 1 (SYSV), statically linked, stripped
```

## Tests

```console
cargo test --target x86_64-unknown-linux-gnu -- --test-threads=1
```

Tests are ran on x64 thanks to `libspeculos.so`.

`--test-threads=1` is required because `libspeculos.so` isn't thread safe.

## Notes

- Bug: LTO and codegen-units seems to be broken, that's why they're commented out in `Cargo.toml`. The generated code is invalid.
- Find which functions take the most of space with `cargo install bloat && cargo bloat --release -n 10`
