# Dockerfile to build RISC-V rust binaries
#
# docker build -t rust -f rust.Dockerfile .

FROM rust:latest

RUN rustup default nightly
RUN rustup target add --toolchain=nightly riscv32i-unknown-none-elf
RUN rustup component add clippy
RUN rustup component add rustfmt
