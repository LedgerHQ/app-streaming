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

Using speculos:

```console
$ speculos.py --model nanox --sdk 2.0.2 vm/bin/app.elf &
$ ./host/stream.py --speculos --app ./app/build/app-swap/app-swap
```

Or using a real device:

```console
$ ./host/stream.py --app ./app/build/app-swap/app-swap
```

Requests can then be entered in hexadecimal on stdin. Clients are also
available, for instance for app-swap:

```console
$ ./app/app-swap/swap.py --speculos --app ./app/build/app-swap/app-swap
```

Once the app is signed, the `.zip` can be passed as an argument:

```console
$ ./app/app-swap/swap.py --speculos --app /tmp/app.zip
```

### Sign the app

While everything is done transparently when passing an ELF file to `stream.py`,
the following commands can be used to sign an app manually. The app is first
signed by a (fake) Ledger HSM, then by the device:

```console
$ python host/hsm.py --elf-path app/build/app-ethereum/app-ethereum --app-path /tmp/app.zip
$ python host/app.py --speculos --app-path /tmp/app.zip
```
