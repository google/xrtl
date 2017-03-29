// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XRTL_TOOLS_TARGET_PLATFORM_TARGET_PLATFORM_H_
#define XRTL_TOOLS_TARGET_PLATFORM_TARGET_PLATFORM_H_

// The bazel rule defines one of the following top-level platforms and then
// one platform+architecture pair for that platform.
//
// XRTL_ARCH_ARM
// XRTL_ARCH_ARM64
// XRTL_ARCH_ASMJS
// XRTL_ARCH_WASM
// XRTL_ARCH_X86
// XRTL_ARCH_X86_64
//
// XRTL_PLATFORM_ANDROID
// XRTL_PLATFORM_ANDROID_EMULATOR
// XRTL_PLATFORM_ANDROID_ARM
// XRTL_PLATFORM_ANDROID_ARM64
// XRTL_PLATFORM_ANDROID_X86
// XRTL_PLATFORM_ANDROID_X86_64
//
// XRTL_PLATFORM_EMSCRIPTEN
// XRTL_PLATFORM_EMSCRIPTEN_ASMJS
// XRTL_PLATFORM_EMSCRIPTEN_WASM
//
// XRTL_PLATFORM_IOS
// XRTL_PLATFORM_IOS_SIMULATOR
// XRTL_PLATFORM_IOS_ARM
// XRTL_PLATFORM_IOS_ARM64
// XRTL_PLATFORM_IOS_X86
// XRTL_PLATFORM_IOS_X86_64
//
// XRTL_PLATFORM_LINUX
// XRTL_PLATFORM_LINUX_X86_64
//
// XRTL_PLATFORM_MACOS
// XRTL_PLATFORM_MACOS_X86_64
//
// XRTL_PLATFORM_WINDOWS
// XRTL_PLATFORM_WINDOWS_X86_64
//
// The special define XRTL_PLATFORM_GOOGLE will be specified if the build
// is being performed within the internal Google repository.

#endif  // XRTL_TOOLS_TARGET_PLATFORM_TARGET_PLATFORM_H_