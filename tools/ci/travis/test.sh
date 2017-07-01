#!/bin/bash
set -e

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  export CC=clang-4.0
  export CXX=clang++-4.0
else
  export CC=clang
  export CXX=clang
fi

# Use CI-specific bazel configurations.
cp tools/ci/travis/.bazelrc .

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  # TODO(benvanik): figure out how to get x running correctly.
  ./xtool test --all --keep_going --test_tag_filters=-requires_gui,-requires_gpu
else
  ./xtool test --all --keep_going --test_tag_filters=-requires_gui,-requires_gpu
fi
