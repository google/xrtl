#!/bin/bash
set -e

export CC=clang-4.0
export CXX=clang++-4.0

# Use Jenkins-specific bazel configurations.
cp tools/ci/jenkins/linux/.bazelrc .

./xtool setup

# TODO(benvanik): setup xtool to use output_base for tidy
# echo "Running tidy and writing to $WORKSPACE/tidy/"
# ./xtool tidy --output_base=$WORKSPACE/tidy/ --origin
./xtool tidy --origin
