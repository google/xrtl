#include "xrtl/base/logging.h"

#include <cstdlib>
#include <vector>

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
