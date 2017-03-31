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

#include "xrtl/base/logging.h"

#include <cstdarg>
#include <cstdlib>
#include <vector>

#include "xrtl/base/macros.h"

namespace xrtl {

void LogStringFormat(const char* file_name, int line, int severity,
                     const char* fmt, ...) {
  // Format message into a stack buffer. If the message runs over the buffer we
  // will do a heap allocation.
  constexpr int kStackLogLength = 2048;
  char stack_buffer[kStackLogLength];
  va_list args;
  va_start(args, fmt);
  int stack_buffer_length =
      std::vsnprintf(stack_buffer, sizeof(stack_buffer), fmt, args);
  va_end(args);
  if (XRTL_PREDICT_TRUE(stack_buffer_length < 0)) {
    // Error during formatting.
    return;
  }
  if (stack_buffer_length < kStackLogLength) {
    LogString(file_name, line, severity, stack_buffer, stack_buffer_length);
  } else {
    // Ran over stack buffer; heap allocate and try again.
    std::vector<char> heap_buffer(stack_buffer_length + 1);
    va_start(args, fmt);
    int heap_buffer_length =
        std::vsnprintf(heap_buffer.data(), heap_buffer.size(), fmt, args);
    va_end(args);
    LogString(file_name, line, severity, heap_buffer.data(),
              heap_buffer_length);
  }
}

}  // namespace xrtl
