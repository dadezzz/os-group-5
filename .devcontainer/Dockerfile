FROM docker.io/library/ubuntu:24.04@sha256:ac3a3d13e90c660a1cb3e89ec695533399f7378dc1ae5fbdc28eb5781d561594

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
