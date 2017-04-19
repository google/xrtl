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

#ifndef XRTL_TOOLS_TARGET_CONFIG_TARGET_CONFIG_H_
#define XRTL_TOOLS_TARGET_CONFIG_TARGET_CONFIG_H_

// The bazel rule defines one of the following top-level configurations:
// XRTL_CONFIG_DBG
// XRTL_CONFIG_FASTBUILD
// XRTL_CONFIG_OPT
//
// Additionally one or more of these may be defined:
// XRTL_CONFIG_GOOGLE_INTERNAL
// XRTL_CONFIG_LOGGING_VERBOSE
//
// Depending on instrumentation mode, we set these dynamically:
// XRTL_CONFIG_ASAN
// XRTL_CONFIG_MSAN
// XRTL_CONFIG_TSAN

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define XRTL_CONFIG_ASAN 1
#endif  // __has_feature(address_sanitizer)
#if __has_feature(memory_sanitizer)
#define XRTL_CONFIG_MSAN 1
#endif  // __has_feature(memory_sanitizer)
#if __has_feature(thread_sanitizer)
#define XRTL_CONFIG_TSAN 1
#endif  // __has_feature(thread_sanitizer)
#endif  // __has_feature

#endif  // XRTL_TOOLS_TARGET_CONFIG_TARGET_CONFIG_H_
