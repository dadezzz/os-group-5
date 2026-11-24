FROM docker.io/library/ubuntu:24.04@sha256:562111b88caa79c71391c0c4d3c2114a7d5c35915c20561fd893cb1e07929e6b

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
