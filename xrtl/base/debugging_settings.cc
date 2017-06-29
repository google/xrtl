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

#include "xrtl/base/debugging_settings.h"

namespace xrtl {
namespace debugging {

// These options functions are used by the --config=asan/msan/etc settings.

// The callbacks we define here will be called from the sanitizer runtime, but
// aren't referenced from the leaf executables. We must ensure that those
// callbacks are not sanitizer-instrumented, and that they aren't stripped by
// the linker.
#define SANITIZER_HOOK_ATTRIBUTE                                              \
  extern "C" __attribute__((no_sanitize_address))                             \
      __attribute__((no_sanitize_memory)) __attribute__((no_sanitize_thread)) \
          __attribute__((visibility("default"))) __attribute__((used))

#if defined(ASAN_OPTIONS)
SANITIZER_HOOK_ATTRIBUTE const char* __asan_default_options() {
  return ASAN_OPTIONS;
}
#endif  // ASAN_OPTIONS

#if defined(LSAN_OPTIONS)
SANITIZER_HOOK_ATTRIBUTE const char* __lsan_default_options() {
  return LSAN_OPTIONS;
}
SANITIZER_HOOK_ATTRIBUTE const char* __lsan_default_suppressions() {
  // Each line must end with a \n.
  // More information:
  // http://dev.chromium.org/developers/testing/leaksanitizer
  return
      // Swiftshader leaks a bit.
      "leak:libEGL.so\n"
      "leak:libGLESv2.so\n"
      //"leak:sw::*\n"
      //"leak:Ice::*\n"
      //"leak:eglQueryString\n"
      //"leak:eglMakeCurrent\n"

      // End.
      "";
}
#endif  // LSAN_OPTIONS

#if defined(MSAN_OPTIONS)
SANITIZER_HOOK_ATTRIBUTE const char* __msan_default_options() {
  return MSAN_OPTIONS;
}
#endif  // MSAN_OPTIONS

#if defined(TSAN_OPTIONS)
SANITIZER_HOOK_ATTRIBUTE const char* __tsan_default_options() {
  return TSAN_OPTIONS;
}
#endif  // TSAN_OPTIONS

#if defined(UBSAN_OPTIONS)
SANITIZER_HOOK_ATTRIBUTE const char* __ubsan_default_options() {
  return UBSAN_OPTIONS;
}
#endif  // UBSAN_OPTIONS

}  // namespace debugging
}  // namespace xrtl
