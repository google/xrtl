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
  # TODO(benvanik): setup xtool to use output_base for tidy
  # echo "Running tidy and writing to $WORKSPACE/tidy/"
  # ./xtool tidy --output_base=$WORKSPACE/tidy/ --origin
  ./xtool tidy --origin
else
  echo "(skipping analysis on osx)"
fi
