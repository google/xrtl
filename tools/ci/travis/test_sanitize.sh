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
cp tools/ci/travis/test_sanitize.bazelrc .bazelrc

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  echo "Running $1 and writing to sanitize-$1/"
  ./xtool test \
      --output_base=/tmp/.cache/bazel_root_sanitize_$1/ \
      --config=$1 \
      --keep_going \
      --all \
      --test_tag_filters=-requires_gui,-requires_gpu
else
  echo "(skipping $1 sanitize test on osx)"
fi
