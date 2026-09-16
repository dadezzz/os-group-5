FROM docker.io/library/ubuntu:24.04@sha256:b35f8c42edf2f1c13abcb5434d8c2034a3f174a517cf0ca71ff3a1acb8a23640

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
