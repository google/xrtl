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

#include <sstream>

#if defined(XRTL_PLATFORM_APPLE) || defined(XRTL_PLATFORM_LINUX)
#define HAS_BACKTRACE
#include <execinfo.h>
#include <unistd.h>
#endif  // APPLE || LINUX

namespace xrtl {
namespace debugging {

#if defined(HAS_BACKTRACE)
std::string CaptureStackTraceString() {
  constexpr int kStackBufferSize = 64;
  void* stack_buffer[kStackBufferSize] = {0};
  int stack_depth = backtrace(stack_buffer, kStackBufferSize);
  if (stack_depth <= 1) {
    return "";
  }
  char** stack_strings = backtrace_symbols(stack_buffer, stack_depth);
  if (!stack_strings) {
    return "";
  }
  std::ostringstream ss;
  ss << "Stack:\n";
  for (int i = 1; i < stack_depth; ++i) {
    ss << stack_strings[i] << '\n';
  }
  std::free(stack_strings);
  return ss.str();
}
#else
std::string CaptureStackTraceString() { return ""; }
#endif  // HAS_BACKTRACE

}  // namespace debugging
}  // namespace xrtl
