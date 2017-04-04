#!/bin/bash
set -e

BAZEL_VERSION=0.4.5
OS=linux
ARCH=x86_64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then OS=darwin; fi

GH_BASE="https://github.com/bazelbuild/bazel/releases/download/$BAZEL_VERSION"
GH_ARTIFACT="bazel-$BAZEL_VERSION-installer-$OS-$ARCH.sh"
BAZEL_URL="$GH_BASE/$GH_ARTIFACT"
echo $BAZEL_URL

wget -O install.sh $BAZEL_URL
chmod +x install.sh
./install.sh --user
rm -f install.sh

LLVM_VERSION=4.0.0
LLVM_PLATFORM=x86_64-linux-gnu-ubuntu-14.04
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  LLVM_PLATFORM=x86_64-apple-darwin
fi
LLVM_ARTIFACT=clang+llvm-$LLVM_VERSION-$LLVM_PLATFORM
LLVM_URL="http://releases.llvm.org/$LLVM_VERSION/$LLVM_ARTIFACT.tar.xz"
wget -O llvm.tar.xz $LLVM_URL
tar -xf llvm.tar.xz
rm llvm.tar.xz
mv $LLVM_ARTIFACT llvm

pip install protobuf
