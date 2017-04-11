#!/bin/bash
set -e

export CC=clang-4.0
export CXX=clang++-4.0

# Use Jenkins-specific bazel configurations.
cp tools/ci/jenkins/linux/.bazelrc .

echo "Running $1 and writing to $WORKSPACE/sanitize-$1/"
./xtool test --output_base=$WORKSPACE/sanitize-$1/ --config=$1 //xrtl/base/...
