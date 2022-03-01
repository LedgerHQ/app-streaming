How to build native apps.

## Setup

Build the docker image:

```console
docker build -t native -f native.Dockerfile .
```

Configure CMake to build the native binaries into `build/native/`:
```
SPECULOS_DIR=/speculos cmake -Bbuild/native/ -H. -DNATIVE=1
```

## Build

Build everything:

```console
SPECULOS_DIR=/speculos make -C build/native/
```

Launch and communicate with a native binary:

```console
../host/client.py --app build/native/app-ethereum/app-ethereum --plugin app-ethereum/plugin-ethereum.py
```

## Tests

```console
make -C build/native/ test
```
