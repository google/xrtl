Cross-platform Real-Time Rendering Library
==========================================

[![Build Status](https://35.185.235.106/buildStatus/icon?job=XRTL%20TAP/master&style=plastic)](https://35.185.235.106/blue/organizations/jenkins/XRTL%20TAP/activity)

A lightweight framework for writing efficient cross-platform rendering code in
modern C++. It abstracts operating system and toolchain concepts to enable quick
prototyping and interation of rich interactive applications that run beautifully
everywhere. The library is engineered for small code sizes to enable fast
loading and low resource usage even within web browsers.

Pronounced 'zurtle'.

*This is not an official Google product.*

## Setup Instructions

### MacOS

* Install clang (and clang-tools-extra) from the [LLVM downloads page](http://releases.llvm.org/download.html)
* Install bazel via [homebrew](https://brew.sh/) or the [bazel releases page](https://github.com/bazelbuild/bazel/releases):
```
$ sudo brew install bazel
```
* Run initial setup with xtool:
```
$ ./xtool setup
```

### Ubuntu

* Install clang (and clang-tools-extra) from the [LLVM downloads page](http://releases.llvm.org/download.html)
* Install bazel via your package manager or the [bazel releases page](https://github.com/bazelbuild/bazel/releases):
```
$ sudo apt-get install bazel
```
* Run initial setup with xtool:
```
$ ./xtool setup
```

### Windows

* Install clang (and clang-tools-extra) from the [LLVM downloads page](http://releases.llvm.org/download.html)
* Install Visual Studio 2015 or 2017 with the Windows SDK and C++ tools
* Install bazel via [chocolatey](https://chocolatey.org/) or the [bazel releases page](https://github.com/bazelbuild/bazel/releases):
```
> choco install bazel
```
* Follow the [bazel on Windows](https://bazel.build/versions/master/docs/windows.html) instructions (set `BAZEL_SH`, etc)
* Run initial setup with xtool:
```
> xtool setup
```
* Generate a Visual Studio solution; do this each time you change BUILD files:
```
> xtool sln
```
* Open `.vs\xrtl.sln`, select a project, and press `F5`!

## Developing

* Build/test/run/query for host platform:
```
$ ./xtool build //xrtl/base/...
$ ./xtool test //xrtl/base/...
$ ./xtool run //xrtl/base:math_test -- --some_arg
# Note that you can use `bazel` instead of `xtool` in most cases, but you must
# specify a config.
$ bazel build --config=macos_x86_64 //xrtl/base/...
```

* Run linter and fix common errors:
```
$ ./xtool fix
```

## Committing Code

* Run presubmit to perform lint/style check/run tests.
```
$ ./xtool presubmit --fix
```
