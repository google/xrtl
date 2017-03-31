#!/bin/bash
set -e

# Use travis-specific bazel configurations.
cp tools/ci/travis/.bazelrc .

export PATH=$PWD/llvm/bin/:$PATH

export CC=clang-4.0
export CXX=clang++-4.0

# xtool presubmit, which should give us lint/style check/etc.
./xtool presubmit
