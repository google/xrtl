Cross-platform Real-Time Rendering Library
==========================================

| **`Linux/Mac CPU`** | **`Windows CPU`** |
|---------------------|-------------------|
| [![Build Status](https://travis-ci.org/google/xrtl.svg?branch=master)](https://travis-ci.org/google/xrtl) | [![Build status](https://ci.appveyor.com/api/projects/status/wp1p760x93tw1p1d/branch/master?svg=true)](https://ci.appveyor.com/project/benvanik/xrtl/branch/master) |

**Status**: currently bringing up the foundations of the library and massaging code from previous internal projects to something acceptable for public release. Please be patient!

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
$ brew install bazel go
```
* Install protobuf:
```
$ sudo easy_install protobuf
```
* Run initial setup with xtool:
```
$ ./xtool setup
```

### Ubuntu

* Install clang (and clang-tools-extra) from the [LLVM downloads page](http://releases.llvm.org/download.html)
* Install bazel via your package manager or the [bazel releases page](https://github.com/bazelbuild/bazel/releases):
```
$ sudo apt-get install bazel golang-go
```
* Run initial setup with xtool:
```
$ ./xtool setup
```

### Windows

* Install clang (and clang-tools-extra) from the [LLVM downloads page](http://releases.llvm.org/download.html)
* Install Visual Studio 2017 with the Windows SDK and C++ tools
* Install bazel via [chocolatey](https://chocolatey.org/) or the [bazel releases page](https://github.com/bazelbuild/bazel/releases):
```
> choco install bazel golang
```
* Follow the [bazel on Windows](https://bazel.build/versions/master/docs/windows.html) instructions (set `BAZEL_SH`, etc)
* Run initial setup with xtool:
```
> xtool setup
```

## IDE Setup

### Visual Studio 2017

* Generate a Visual Studio solution; do this each time you change BUILD files:
```
> xtool sln -c dbg
```
* Open `.vs\xrtl.sln`, select a project, and press `F5`!

### CLion

* Install the 'CLion with Bazel' plugin from the repository.
* Import the bazel project from the root git folder.
* Set the bazel binary to `xtool` (or `xtool.bat` on Windows).
* Import the project view from the workspace `.bazelproject` file.

If clang is not your default CC you should start CLion with CC=clang.

## Developing

* Build/test/run/query for host platform:
```
$ ./xtool build --all
$ ./xtool test --all
$ ./xtool run //xrtl/base:math_test -- --some_arg
# Note that you can use `bazel` instead of `xtool` in most cases, but you must
# specify a config.
$ bazel build --config=macos_x86_64 //xrtl/base/...
```

* Run linter and fix common errors:
```
$ ./xtool fix
```

### Debugging Notes

GDB may need the following commands run at startup to resolve some third party
source code. Replace directories with your workspace location:
```
dir $HOME/xrtl/bazel-xrtl
set substitute-path external/com_github_google_swiftshader $HOME/xrtl/third_party/swiftshader/
set substitute-path external/com_github_khronosgroup_glslang $HOME/xrtl/third_party/glslang/
```

## Committing Code

* Run presubmit to perform lint/style check/run tests.
```
$ ./xtool presubmit --fix
```

## Testing

### Test Tags

Common `tags` that can be added to tests:

* `exclusive`: no other tests should run while this test is executing.
* `requires_gui`: test uses a GUI and requires a window manager (X11/etc).
* `requires_gpu`: test requires a real hardware GPU to run.

```
# Run all tests except those that require a hardware GPU:
$ ./xtool test --all --test_tag_filter=-requires_gpu
```
