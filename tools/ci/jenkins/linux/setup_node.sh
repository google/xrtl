#!/bin/bash
set -e

# Setup for new Linux Jenkins nodes.
# This requires an instance with:
#   - Debian jessie
#   - Boot disk as SSD with >=100GB
#   - K80 GPU (optional)
#
# Once the instance has been setup you can ssh in and run this script.

# Install required deps for install.
sudo apt update -y
sudo apt upgrade -y
sudo apt install -y \
    curl \
    git \
    subversion \
    wget \
    python-pip

# Install Python modules.
sudo pip install protobuf

# Configure automatic updates.
sudo apt install -y \
    unattended-upgrades \
    apt-listchanges
sudo dpkg-reconfigure -plow unattended-upgrades
# Can change this to choose what is upgraded:
#   /etc/apt/apt.conf.d/50unattended-upgrades

# Install K80 GPU drivers.
# https://wiki.debian.org/NvidiaGraphicsDrivers#Debian_8_.22Jessie.22
echo "deb http://httpredir.debian.org/debian jessie-backports main contrib non-free" |
    sudo tee /etc/apt/sources.list.d/backports.list
sudo apt update -y
sudo apt install -y -t jessie-backports \
    linux-headers-$(uname -r|sed 's,[^-]*-[^-]*-,,') \
    nvidia-detect \
    nvidia-driver

# Install Java JDK.
# Note that we need to pull the new CA certificates from backports else the
# install will fail.
sudo apt upgrade -y -t jessie-backports ca-certificates-java
sudo apt install -y openjdk-8-jdk
sudo update-alternatives --set java /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java

# Install Bazel.
echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | \
    sudo tee /etc/apt/sources.list.d/bazel.list
wget -O - https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
sudo apt update -y
sudo apt install -y bazel

# Install LLVM.
echo "deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie main
deb-src http://apt.llvm.org/jessie/ llvm-toolchain-jessie main
deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-3.9 main
deb-src http://apt.llvm.org/jessie/ llvm-toolchain-jessie-3.9 main
deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-4.0 main
deb-src http://apt.llvm.org/jessie/ llvm-toolchain-jessie-4.0 main" | \
    sudo tee /etc/apt/sources.list.d/llvm.list
wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt update -y
sudo apt install -y \
    clang-4.0 \
    lldb-4.0 \
    llvm-4.0 \
    clang-format-4.0 \
    clang-tidy-4.0 \
    lld-4.0

# Install a modern cmake.
wget -O cmake.sh https://cmake.org/files/v3.8/cmake-3.8.0-Linux-x86_64.sh
chmod +x cmake.sh
./cmake.sh --prefix=/usr/local/
rm cmake.sh

# Setup custom msan LLVM.
sudo mkdir -p /usr/local/llvm_scratch/
sudo chmod +rx /usr/local/llvm_scratch/
svn co http://llvm.org/svn/llvm-project/llvm/trunk ~/llvm
(cd ~/llvm/projects && svn co http://llvm.org/svn/llvm-project/libcxx/trunk libcxx)
(cd ~/llvm/projects && svn co http://llvm.org/svn/llvm-project/libcxxabi/trunk libcxxabi)
mkdir ~/libcxx_msan
pushd ~/libcxx_msan
cmake ../llvm -DCMAKE_BUILD_TYPE=Release -DLLVM_USE_SANITIZER=Memory -DCMAKE_C_COMPILER=clang-4.0 -DCMAKE_CXX_COMPILER=clang++-4.0
make cxx -j12
popd
sudo mv libcxx_msan/ /usr/local/llvm_scratch/libcxx_msan/

# Setup Jenkins user.
sudo adduser --system --group \
    --home=/var/lib/jenkins-node/ --no-create-home \
    --disabled-password \
    --shell /bin/bash \
    --quiet \
    jenkins-node
sudo install -d -o jenkins-node -g jenkins-node /var/lib/jenkins-node
install -d -m 700 -o jenkins-node -g jenkins-node /var/lib/jenkins-node/.ssh
# copy
#    /var/lib/jenkins/.ssh/id_rsa.pub
#    /var/lib/jenkins-node/.ssh/authorized_keys
# test from master: sudo -u jenkins -H ssh jenkins-node@IP
