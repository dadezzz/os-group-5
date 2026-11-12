FROM docker.io/library/ubuntu:24.04@sha256:2198e4c65076e9cb0b3d695219df667c0e3b0072da64ca3c2648aca17dcecaf5

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
