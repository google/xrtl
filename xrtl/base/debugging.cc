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

#include "xrtl/base/debugging.h"

namespace xrtl {
namespace debugging {

// These options functions are used by the --config=asan/msan/etc settings.

#if defined(ASAN_OPTIONS)
extern "C" const char *__asan_default_options() {
  return ASAN_OPTIONS;
}
#endif  // ASAN_OPTIONS

#if defined(LSAN_OPTIONS)
extern "C" const char *__lsan_default_options() {
  return "report_objects=1";
}
#endif  // LSAN_OPTIONS

#if defined(MSAN_OPTIONS)
extern "C" const char *__msan_default_options() {
  return "";
}
#endif  // MSAN_OPTIONS

#if defined(TSAN_OPTIONS)
extern "C" const char *__tsan_default_options() {
  return "";
}
#endif  // TSAN_OPTIONS

#if defined(UBSAN_OPTIONS)
extern "C" const char *__ubsan_default_options() {
  return "";
}
#endif  // UBSAN_OPTIONS

}  // namespace debugging
}  // namespace xrtl
