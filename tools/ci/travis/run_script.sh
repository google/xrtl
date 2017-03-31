#!/bin/sh
set -e

# Return to root.
cd ../../..

# Use travis-specific bazel configurations.
cp ./tools/ci/travis/.bazelrc .

# xtool presubmit, which should give us lint/style check/etc.
./xtool
