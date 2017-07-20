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
// XRTL_ARCH_ARM_V7A
// XRTL_ARCH_ARM_V8A
// XRTL_ARCH_ASMJS
// XRTL_ARCH_WASM_32
// XRTL_ARCH_WASM_64
// XRTL_ARCH_X86_32
// XRTL_ARCH_X86_64
//
// TODO(benvanik): endianness
//
// XRTL_COMPILER_CLANG
// XRTL_COMPILER_GCC
// XRTL_COMPILER_GCC_COMPAT
// XRTL_COMPILER_MSVC
//
// XRTL_PLATFORM_ANDROID
// XRTL_PLATFORM_ANDROID_EMULATOR
// XRTL_PLATFORM_APPLE (IOS | MACOS)
// XRTL_PLATFORM_EMSCRIPTEN
// XRTL_PLATFORM_IOS
// XRTL_PLATFORM_IOS_SIMULATOR
// XRTL_PLATFORM_LINUX
// XRTL_PLATFORM_MACOS
// XRTL_PLATFORM_WINDOWS
//
// The special define XRTL_PLATFORM_GOOGLE will be specified if the build
// is being performed within the internal Google repository.

//==============================================================================
// XRTL_ARCH_*
//==============================================================================

#if defined(__arm__) || defined(__arm64) || defined(__aarch64__) || \
    defined(__thumb__) || defined(__TARGET_ARCH_ARM) ||             \
    defined(__TARGET_ARCH_THUMB) || defined(_M_ARM)
#if defined(__arm64) || defined(__aarch64__)
#define XRTL_ARCH_ARM_V8A 1
#else
#define XRTL_ARCH_ARM_V7A 1
#endif  // __arm64
#endif  // ARM

#if defined(__wasm32__)
#define XRTL_ARCH_WASM_32 1
#elif defined(__wasm64__)
#define XRTL_ARCH_WASM_64 1
#elif defined(__asmjs__)
#define XRTL_ARCH_ASMJS 1
#endif  // wasm/asmjs

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || \
    defined(__i686__) || defined(__i386) || defined(_M_IX86) || defined(_X86_)
#define XRTL_ARCH_X86_32 1
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__amd64) || defined(_M_X64)
#define XRTL_ARCH_X86_64 1
#endif  // X86

#if !defined(XRTL_ARCH_ARM_V7A) && !defined(XRTL_ARCH_ARM_V8A) && \
    !defined(XRTL_ARCH_ASMJS) && !defined(XRTL_ARCH_WASM_32) &&   \
    !defined(XRTL_ARCH_WASM_64) && !defined(XRTL_ARCH_X86_32) &&  \
    !defined(XRTL_ARCH_X86_64)
#error Unknown architecture.
#endif  // all archs

//==============================================================================
// XRTL_COMPILER_*
//==============================================================================

#if defined(__clang__)
#define XRTL_COMPILER_CLANG 1
#define XRTL_COMPILER_GCC_COMPAT 1
#elif defined(__GNUC__)
#define XRTL_COMPILER_GCC 1
#define XRTL_COMPILER_GCC_COMPAT 1
#elif defined(_MSC_VER)
#define XRTL_COMPILER_MSVC 1
#else
#error Unrecognized compiler.
#endif  // compiler versions

//==============================================================================
// XRTL_PLATFORM_ANDROID
//==============================================================================

#if defined(__ANDROID__)
#define XRTL_PLATFORM_ANDROID 1
#endif  // __ANDROID__

//==============================================================================
// XRTL_PLATFORM_EMSCRIPTEN
//==============================================================================

#if defined(__EMSCRIPTEN__)
#define XRTL_PLATFORM_EMSCRIPTEN 1
#endif  // __ANDROID__

//==============================================================================
// XRTL_PLATFORM_IOS | XRTL_PLATFORM_MACOS
//==============================================================================

#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define XRTL_PLATFORM_IOS 1
#else
#define XRTL_PLATFORM_MACOS 1
#endif  // TARGET_OS_IPHONE
#if TARGET_IPHONE_SIMULATOR
#define XRTL_PLATFORM_IOS_SIMULATOR 1
#endif  // TARGET_IPHONE_SIMULATOR
#endif  // __APPLE__

//==============================================================================
// XRTL_PLATFORM_LINUX
//==============================================================================

#if defined(__linux__) || defined(linux) || defined(__linux)
#define XRTL_PLATFORM_LINUX 1
#endif  // __linux__

//==============================================================================
// XRTL_PLATFORM_WINDOWS
//==============================================================================

#if defined(_WIN32) || defined(_WIN64)
#define XRTL_PLATFORM_WINDOWS 1
#endif  // _WIN32 || _WIN64

#if defined(XRTL_PLATFORM_IOS) || defined(XRTL_PLATFORM_MACOS)
#define XRTL_PLATFORM_APPLE 1
#endif  // XRTL_PLATFORM_IOS || XRTL_PLATFORM_MACOS

#if !defined(XRTL_PLATFORM_ANDROID) && !defined(XRTL_PLATFORM_EMSCRIPTEN) && \
    !defined(XRTL_PLATFORM_IOS) && !defined(XRTL_PLATFORM_LINUX) &&          \
    !defined(XRTL_PLATFORM_MACOS) && !defined(XRTL_PLATFORM_WINDOWS)
#error Unknown platform.
#endif  // all archs

#endif  // XRTL_TOOLS_TARGET_PLATFORM_TARGET_PLATFORM_H_
