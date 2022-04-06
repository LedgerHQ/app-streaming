## How-To

### Build a RISC-V app

Build the docker image to have a ready-to-use RISC-V toolchain:

```console
docker build -t riscv .
```

Retrieve jsmn and nanopb git submodule:
```console
git submodule update --init
```

Build the RISC-V app using the `docker.sh` script:

```console
$ ./docker.sh riscv
[root:/app] # cmake -Bbuild -H.
[root:/app] # make -C build/
```

### Build the RISC-V VM and install it on the Nano device

Like any other Nano apps. Sources are in the `vm/` directory.

```console
cd vm/
make
make load
```

### Run the app

Using speculos (2.0.0):

```console
$ speculos.py --model nanox --display headless vm/bin/app.elf &
$ ./host/stream.py --speculos --verbose --app ./app/build/app-wip/app
```

Or using a real device (2.0.2):

```console
$ ./host/stream.py --verbose --app ./app/build/app-wip/app
```

### Sign the app

While everything is done transparently when passing an ELF file to `stream.py`,
the following commands can be used to sign an app manually. The app is first
signed by a (fake) Ledger HSM, then by the device:

```shell
python host/hsm.py --elf-path app/build/app-ethereum/app-ethereum --app-path /tmp/app.zip
python host/app.py --speculos --app-path /tmp/app.zip
```
