#!/bin/bash
set -e

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  export CC=clang-4.0
  export CXX=clang++-4.0
else
  export CC=clang
  export CXX=clang
fi

# Ensure we have origin/master, which our diff tools use.
git fetch https://github.com/google/xrtl.git master:refs/remotes/origin/master

# Fetch a modern LLVM on osx. On linux travis.yml will apt install it.
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  echo "Fetching LLVM..."
  HOMEBREW_NO_AUTO_UPDATE=1 brew install https://raw.githubusercontent.com/Homebrew/homebrew-core/master/Formula/llvm.rb
  HOMEBREW_NO_AUTO_UPDATE=1 brew install https://raw.githubusercontent.com/Homebrew/homebrew-core/master/Formula/clang-format.rb
fi

# Use CI-specific bazel configurations.
cp tools/ci/travis/.bazelrc .

# Fetch the Bazel installer.
# Assumes the following environment variables are set in CI config:
# - BAZEL_VERSION=0.5.2
# - BAZEL_INSTALLER=bazel-$BAZEL_VERSION-jdk7-installer-linux-x86_64.sh
echo "Fetching bazel..."
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  BAZEL_INSTALLER_URL=https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-installer-darwin-x86_64.sh
else
  BAZEL_INSTALLER_URL=https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-jdk7-installer-linux-x86_64.sh
fi
BAZEL_INSTALLER_SH=${HOME}/bazel-install.sh
curl -L -o ${BAZEL_INSTALLER_SH} ${BAZEL_INSTALLER_URL}

# Install bazel to $HOME/bin.
echo "Installing bazel..."
bash "${BAZEL_INSTALLER_SH}" --user

# Fetch git-clang-format.
echo "Fetching git-clang-format..."
curl -L -o ${HOME}/bin/git-clang-format https://llvm.org/svn/llvm-project/cfe/trunk/tools/clang-format/git-clang-format
chmod +x ${HOME}/bin/git-clang-format

# TODO(benvanik): figure out how to make this useful.
# Prepare X server.
# if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
#   echo "Launching X..."
#   Xvfb ${DISPLAY} -screen 0 1280x1024x24 > /dev/null 2>&1 &
#   # Wait for xvfb to be ready for connections.
#   X_WAIT_MAX=120 # About 60 seconds
#   X_WAIT_DURATION=0
#   while ! xdpyinfo -display ${DISPLAY} >/dev/null 2>&1; do
#     sleep 0.50s
#     X_WAIT_DURATION=$((X_WAIT_DURATION + 1))
#     if [ "$X_WAIT_DURATION" -ge "$X_WAIT_MAX" ]; then
#       echo "FATAL: $0: gave up waiting for X server $DISPLAY"
#       exit 1
#     fi
#   done
#   echo "X ready on ${DISPLAY} (probably)"
# fi

echo "Running 'xtool setup'..."
./xtool setup
