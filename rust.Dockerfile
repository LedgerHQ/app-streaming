# Dockerfile to build RISC-V rust binaries
#
# docker build -t rust -f rust.Dockerfile .

FROM rust:latest

RUN rustup target add riscv32i-unknown-none-elf
