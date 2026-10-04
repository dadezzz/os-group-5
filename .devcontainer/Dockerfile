FROM docker.io/library/ubuntu:24.04@sha256:534baea6a22c03a63003dbc8dbe78fe34bc0d7e595d9a9dc9834884ff530eb55

ENV DEBIAN_FRONTEND="noninteractive"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        git \
        make \
        clang \
        clang-tools \
        libclang-rt-dev \
        llvm \
        clang-format \
        clang-tidy \
        ca-certificates && \
    apt-get clean && \
    rm -r /var/lib/apt/lists/
