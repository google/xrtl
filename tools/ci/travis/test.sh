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
  CONFIG=asan
  # TODO(benvanik): figure out how to get x running correctly so we can do
  #                 requires_gui tests.
else
  CONFIG=
fi

./xtool test \
    --config=$CONFIG \
    --keep_going \
    --all \
    --test_tag_filters=-requires_gui,-requires_gpu
