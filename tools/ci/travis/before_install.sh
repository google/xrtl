#!/bin/sh
set -e

BAZEL_VERSION=0.4.5
OS=linux
ARCH=x86_64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then OS=darwin; fi

GH_BASE="https://github.com/bazelbuild/bazel/releases/download/$BAZEL_VERSION"
GH_ARTIFACT="bazel-$V-installer-$OS-$ARCH.sh"
CI_BASE="http://ci.bazel.io/job/Bazel/JAVA_VERSION=1.8,PLATFORM_NAME=$OS-$ARCH/lastSuccessfulBuild/artifact/output/ci"
CI_ARTIFACT="bazel--installer.sh"
URL="$GH_BASE/$GH_ARTIFACT"
echo $URL

wget -O install.sh $URL
chmod +x install.sh
./install.sh --user
rm -f install.sh
