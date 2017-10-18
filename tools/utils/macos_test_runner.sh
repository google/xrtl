#!/bin/bash
#
# Required to workaround https://github.com/bazelbuild/bazel/issues/3921.
# If that gets fixed this can be replaced with a --test_env=LD_LIBRARY_PATH
# in bazel.rc. Or, we could fix the underlying reason swiftshader needs this.

export LD_LIBRARY_PATH=$PWD/external/com_github_google_swiftshader/
$@
