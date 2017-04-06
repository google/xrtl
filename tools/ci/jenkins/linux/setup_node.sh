#!/bin/bash
set -e

# Setup for new Linux Jenkins nodes.

sudo apt update

sudo apt install \
    curl \
    wget \
    python-pip

sudo apt upgrade -t jessie-backports ca-certificates-java
sudo apt install openjdk-8-jdk

echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | \
    sudo tee /etc/apt/sources.list.d/bazel.list
wget -O - https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -

echo "deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie main
deb-src http://apt.llvm.org/jessie/ llvm-toolchain-jessie main
deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-3.9 main
deb-src http://apt.llvm.org/jessie/ llvm-toolchain-jessie-3.9 main
deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-4.0 main
deb-src http://apt.llvm.org/jessie/ llvm-toolchain-jessie-4.0 main" | \
    sudo tee /etc/apt/sources.list.d/llvm.list
wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

sudo apt update

sudo apt install \
    bazel \
    clang-4.0 \
    lldb-4.0 \
    llvm-4.0 \
    clang-format-4.0 \
    clang-tidy-4.0 \
    lld-4.0

sudo pip install protobuf
