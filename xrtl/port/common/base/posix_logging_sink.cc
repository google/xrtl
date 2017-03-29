#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "xrtl/base/logging.h"
#include "xrtl/base/system_clock.h"
#include "xrtl/port/common/base/logging_macros.h"

namespace xrtl {

void FlushLog() { std::fflush(stderr); }

namespace internal {

void LogMessage::EmitLogMessage() {
  static SystemClock* clock = SystemClock::logging_clock();
  uint64_t now_micros = clock->now_utc_micros();
  time_t now_seconds = static_cast<time_t>(now_micros / 1000000);
  int32_t micros_remainder = static_cast<int32_t>(now_micros % 1000000);
  constexpr size_t time_buffer_size = 30;
  char time_buffer[time_buffer_size];
  strftime(time_buffer, time_buffer_size, "%Y-%m-%d %H:%M:%S",
           localtime(&now_seconds));

  std::fprintf(stderr, "%s.%06d: %c %s:%d] %s\n", time_buffer, micros_remainder,
               "IWEF"[severity_], file_name_, line_, str().c_str());
}

}  // namespace internal
}  // namespace xrtl
