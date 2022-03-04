# docker build -t native -f native.Dockerfile .

FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get -yq update && \
    apt-get -yq install build-essential clang cmake less make libcmocka-dev libjson-c-dev libprotobuf-c-dev python3 python3-pip wget && \
    pip3 install protobuf

# Unfortunately, libssl-dev has no support for ED25519 on Ubuntu 20.04.
# Build it.

# Create SHA256SUMS, download dependencies and verify their integrity before
# building OpenSSL.

RUN cd /tmp/ && \
  echo 892a0875b9872acd04a9fde79b1f943075d5ea162415de3047c327df33fbaee5 openssl-1.1.1k.tar.gz >> SHA256SUMS && \
  wget --quiet https://www.openssl.org/source/openssl-1.1.1k.tar.gz && \
  sha256sum --check SHA256SUMS && \
  rm SHA256SUMS && \
  tar xf openssl-1.1.1k.tar.gz && \
  cd openssl-1.1.1k && \
  ./Configure no-afalgeng no-aria no-asan no-asm no-async no-autoalginit no-autoerrinit no-autoload-config no-bf no-buildtest-c++ no-camellia no-capieng no-cast no-chacha no-cmac no-cms no-comp no-crypto-mdebug no-crypto-mdebug-backtrace no-ct no-deprecated no-des no-devcryptoeng no-dgram no-dh no-dsa no-dso no-dtls no-ec2m no-ecdh no-egd no-engine no-err no-external-tests no-filenames no-fuzz-afl no-fuzz-libfuzzer no-gost no-heartbeats no-hw no-idea no-makedepend no-md2 no-md4 no-mdc2 no-msan no-multiblock no-nextprotoneg no-ocb no-ocsp no-pinshared no-poly1305 no-posix-io no-psk no-rc2 no-rc4 no-rc5 no-rdrand no-rfc3779 no-scrypt no-sctp no-seed no-shared no-siphash no-sm2 no-sm3 no-sm4 no-sock no-srp no-srtp no-sse2 no-ssl no-ssl3-method no-ssl-trace no-stdio no-tests no-threads no-tls no-ts no-ubsan no-ui-console no-unit-test no-whirlpool no-zlib no-zlib-dynamic no-sock linux-x86_64 && \
  make -j && \
  make install_sw && \
  cd .. && \
  rm -r openssl-1.1.1k/ openssl-1.1.1k.tar.gz

ENTRYPOINT /bin/bash
