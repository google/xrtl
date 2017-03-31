Cross-platform Real-Time Rendering Library
==========================================

A lightweight framework for writing efficient cross-platform rendering code in
modern C++. It abstracts operating system and toolchain concepts to enable quick
prototyping and interation of rich interactive applications that run beautifully
everywhere. The library is engineered for small code sizes to enable fast
loading and low resource usage even within web browsers.

*This is not an official Google product.*

## Setup Instructions

* Install clang (and clang-tools-extra) from the [LLVM downloads page](http://releases.llvm.org/download.html)
* Install bazel via your package manager or the [bazel releases page](https://github.com/bazelbuild/bazel/releases):
```
$ sudo apt-get install bazel
```
* Build (TBD)

### When Developing

* Run linter and fix common errors:
```
$ ./xtool fix
```

### Before Committing Code

* Run presubmit to perform lint/style check/run tests.
```
$ ./xtool presubmit --fix
```
