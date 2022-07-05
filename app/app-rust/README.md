## Rust app for the RISC-V target

### Pre-requisites

Some libraries are required to build a Rust app:

- `libc.a` to build the app for the RISC-V target
- `libcrypto.a` to pass tests on the x64 target

These libraries can be retrieved from Docker images thanks to the script `lib/create-lib.sh`:

```console
./lib/create-lib.sh
```

They are also generated as artifacts by the [GitHub CI](https://github.com/LedgerHQ/app-streaming/actions/workflows/apps.yml).

### Build

```console
cargo build --release
```

### Tests

```console
cargo test --target x86_64-unknown-linux-gnu -- --test-threads=1
```

Tests are ran on x64 thanks to `libspeculos.so`.

`--test-threads=1` is required because `libspeculos.so` isn't thread safe.


## Notes

- Find which functions take the most of space with `cargo install bloat && cargo bloat --release -n 10`
