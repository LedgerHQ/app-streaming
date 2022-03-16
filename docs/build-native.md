How to build native apps.

## Setup

Build the docker image:

```console
docker build -t native -f native.Dockerfile .
```

## Build

From the docker image `./docker.sh native`, configure CMake to build the native binaries into `build/native/`:

```console
cmake -Bbuild/native/ -H. -DNATIVE=1
```

Build everything:

```console
make -C build/native/
```

Launch and communicate with a native binary:

```console
../host/client.py --app build/native/app-ethereum/app-ethereum --plugin app-ethereum/plugin-ethereum.py
```

## Tests

```console
make -C build/native/ test
```
