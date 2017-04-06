#!/bin/bash
set -e

export CC=clang-4.0
export CXX=clang++-4.0

# Use Jenkins-specific bazel configurations.
cp tools/ci/jenkins/linux/.bazelrc .

./xtool setup
./xtool lint
