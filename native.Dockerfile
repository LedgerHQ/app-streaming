# docker build -t native -f native.Dockerfile .

FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get -yq update && \
    apt-get -yq install build-essential cmake make libprotobuf-c-dev python3 python3-pip && pip3 install protobuf

ENTRYPOINT /bin/bash
