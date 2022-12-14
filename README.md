## App Streaming

The *App Streaming* project allows running any app on Ledger Nano devices (X, S+, Stax) without restrictions such as memory limitations. Basically, the Ledger Nano runs a VM which can launch any app ; the (unlimited) memory is exported encrypted on the PC or the smartphone of the user.

For developers, development is now standard since there's no restriction on the stack, heap nor code size. Modern software stack can be used (which implies standard toolchains, tests, fuzzing tools and standard libraries) along usual development patterns. The code is totally independent from the firmware and the same app is compatible with Nano X, Nano S+ and Stax. An emulator such as speculos isn't required anymore and apps can be developed in Rust.

For end-users, it means that there's no restrictions on the number of apps since there's no app install anymore: all supported apps are immediately available. In a similary way, there are no no app updates anymore, app version is always the latest. Regarding the transport, there's no USB or BLE deconnection when switching from an app to another.


### Usage

(Excerpt from [docs/app-dev.md](docs/app-dev.md)).

1. Run the VM with speculos: `speculos.py --model nanox vm/bin/app.elf`
2. Launch the app: `./host/stream.py --speculos --app ./app/build/app-swap/app-swap`


### Code

A few apps are available in the [app/](app/) folder:

- [app/app-ethereum/](app/app-ethereum/) is a subset of the current [Ethereum app](https://github.com/LedgerHQ/app-ethereum), with the addition of EIP-712 support
- [app/app-swap/](app/app-ethereum/) is a subset of the current [Exchange app](https://github.com/LedgerHQ/app-exchange)
- [app/app-rust/](app/app-rust/): also implements some features of the Exchange app, in Rust.
- 2 SDKs are available, a [C SDK](app/sdk/) and a [Rust SDK](app/app-rust/src/sdk/).

The Nano RISC-V VM app is in [vm/](vm/) and Python tools to interact with the VM app are in [host/](host/).

Once the project will be adopted more broadly, it will be split into several repositories. Meanwhile, it's more convenient to work on a mono-repo.

[![Build and test apps](https://github.com/LedgerHQ/app-streaming/actions/workflows/apps.yml/badge.svg)](https://github.com/LedgerHQ/app-streaming/actions/workflows/apps.yml)
[![Build and test the Nano VM](https://github.com/LedgerHQ/app-streaming/actions/workflows/vm.yml/badge.svg)](https://github.com/LedgerHQ/app-streaming/actions/workflows/vm.yml)
[![Build container images](https://github.com/LedgerHQ/app-streaming/actions/workflows/build-packages.yml/badge.svg)](https://github.com/LedgerHQ/app-streaming/actions/workflows/build-packages.yml)


### Documentation

Technical and usage information can be found in the [docs/](docs/) folder.


### Community

Feel free to join the `#app-streaming` Slack channel.
