FROM docker.io/library/ubuntu:24.04

ENV DEBIAN_FRONTEND="noninteractive"

RUN apt-get update && \
    apt-get install -y --no-install-recommends git make clang clang-tools libclang-rt-dev llvm clang-format clang-tidy && \
    apt-get clean && \
    rm -r /var/lib/apt/lists/
