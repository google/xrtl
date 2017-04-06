#!/bin/bash
set -e

export CC=clang-4.0
export CXX=clang++-4.0

# Use Jenkins-specific bazel configurations.
cp tools/ci/jenkins/macos/.bazelrc .

./xtool setup
./xtool build //xrtl/base/...
./xtool test //xrtl/base/...
