FROM docker.io/library/ubuntu:24.04@sha256:d78ab76437b1afc5f01e223d6bf0172763f404bb166441328845adbef44518cb

ENV DEBIAN_FRONTEND="noninteractive"

RUN apt-get update && \
    apt-get install -y --no-install-recommends git make clang clang-tools libclang-rt-dev llvm clang-format clang-tidy && \
    apt-get clean && \
    rm -r /var/lib/apt/lists/
