FROM docker.io/library/ubuntu:26.04@sha256:678c6550cc43645e08669028bc177f50be4e7c5b8cca677067b1914d4afc7a03

ENV DEBIAN_FRONTEND="noninteractive"

RUN apt-get update && \
    apt-get install -y --no-install-recommends git make clang clang-tools libclang-rt-dev llvm clang-format clang-tidy && \
    apt-get clean && \
    rm -r /var/lib/apt/lists/
