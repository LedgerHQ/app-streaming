## Avantages

### For users

#### No more app to install from Ledger Live (ie. no Ledger Live Manager anymore).

Apps aren't installed on the device anymore. When a user want to run an app, this app is *streamed* to the device but isn't stored on it. It means that there's no restriction on the number of apps which can be installed (this concept doesn't exist anymore).

#### Automatic update.

Since apps aren't installed on the device anymore, they are simply fetch from the internet. Once an app is updated, user automatically run the latest version.

#### No USB/BLE deconnection.

Transport (USB, BLE) isn't handled by apps anymore. The USB/BLE stack isn't reset when a new app is launched.


### For app developers

#### App development is made easy. WAY WAY MORE EASIER.

Anyone can now develop an app. For real.

#### There are no specific knowledge to have anymore.

No more `PIC()`. No more misaligned instruction. No more exceptions. No more weird linker scripts.

Standard toolchains can be used. Apps can be developed in memory safe language (eg. Rust) as long as the compiler supports the RISC-V target.

#### There are no memory constraints anymore.

Unlimited RAM. Unlimited heap. Unlimited code size. `malloc()` is now available.

#### Standard libraries can be used.

Since there now is a heap and no memory contraints (and `PIC()` doesn't exist), parsing libraries can be used

#### No weird APDU format.

Since standard libraries can be used, JSON or protobuf can be used to encode messages.

#### App are secure.

Standard development practices make CI, tests, fuzzing easy. Code review is now a thing.

#### Same app for every devices.

Since there's nothing specific to the device, the same app can run on Nano S, Nano X and Nano S+.

#### Apps can be developed on Nano X.

No NDA anymore.

#### Additional stuff.

- No `os_lib_call` anymore since there is no memory constraint anymore. Apps making using of this mechanism can simply embed the *library*.
- No `io_exchange()`.
- New UX library without `io_exchange()`.


## Limitations

#### Overhead.

Especially for BLE? Performances could be increased a bit by removing the IO task.

#### Companinon.

A companion  is always required (Ledger Live Desktop / Mobile).
