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

#ifndef XRTL_BASE_DEBUGGING_H_
#define XRTL_BASE_DEBUGGING_H_

#include <string>

#include "xrtl/base/macros.h"

#if defined(XRTL_CONFIG_ASAN)
#include <sanitizer/lsan_interface.h>
#define XRTL_DISABLE_LEAK_CHECKS() __lsan_disable()
#define XRTL_ENABLE_LEAK_CHECKS() __lsan_enable()
#else
#define XRTL_DISABLE_LEAK_CHECKS()
#define XRTL_ENABLE_LEAK_CHECKS()
#endif  // XRTL_CONFIG_ASAN

namespace xrtl {
namespace debugging {

// Disables leak checking in the scope that contains it.
// This should only be used if absolutely required.
class LeakCheckDisabler {
 public:
  LeakCheckDisabler() { XRTL_DISABLE_LEAK_CHECKS(); }
  ~LeakCheckDisabler() { XRTL_ENABLE_LEAK_CHECKS(); }
};

// Turns on heap debugging features on the platform, if present.
// This may cause asserts and should only be enabled in debug builds.
void EnableDebugHeap();

// Returns true if a user-visible output console is attached.
// If false the user will not see anything written to the logs.
bool is_console_attached();

// Attempts to attach a user-visible output console if not already present.
// Returns true if the console was successfully attached.
bool AttachConsole();

// Returns true if a debugger is currently attached.
// Note that a debugger may be attached at any time.
bool is_debugger_attached();

// Breaks into the debugger if it is currently attached.
// If no debugger is present a signal will be raised and the user will either
// receive a crash or a prompt to attach a debugger.
XRTL_ALWAYS_INLINE void Break() {
// We implement this directly in the header with ALWAYS_INLINE so that the
// stack doesn't get all messed up.
#if defined(XRTL_PLATFORM_WINDOWS)
  __debugbreak();
#elif defined(XRTL_COMPILER_CLANG)
  // TODO(benvanik): test and make sure this works everywhere. It's clang
  //                 builtin but only definitely works on OSX.
  __builtin_debugtrap();
#elif defined(XRTL_ARCH_ARM_V7A)
  __asm__ volatile(".inst 0xe7f001f0");
#elif defined(XRTL_ARCH_ARM_V8A)
  __asm__ volatile(".inst 0xd4200000");
#elif defined(XRTL_ARCH_X86_32) || defined(XRTL_ARCH_X86_64)
  __asm__ volatile("int $0x03");
#elif defined(XRTL_ARCH_EMSCRIPTEN)
  EM_ASM({ debugger; });
#else
  // NOTE: this is unrecoverable and debugging cannot continue.
  __builtin_trap();
#endif  // XRTL_PLATFORM_WINDOWS
}

// Returns a multi-line string containing a stack trace.
// May be a no-op on some platforms and return empty string.
std::string CaptureStackTraceString();

}  // namespace debugging
}  // namespace xrtl

#endif  // XRTL_BASE_DEBUGGING_H_
