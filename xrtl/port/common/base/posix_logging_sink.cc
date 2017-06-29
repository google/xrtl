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

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "xrtl/base/flags.h"
#include "xrtl/base/logging.h"
#include "xrtl/base/system_clock.h"
#include "xrtl/port/common/base/logging_macros.h"

DEFINE_bool(logtostderr, false, "Logs to stderr instead of stdout");

namespace xrtl {

void FlushLog() { std::fflush(FLAGS_logtostderr ? stderr : stdout); }

namespace internal {

void LogMessage::EmitLogMessage() {
  static SystemClock* clock = SystemClock::logging_clock();
  uint64_t now_micros = clock->now_utc_micros().count();
  time_t now_seconds = static_cast<time_t>(now_micros / 1000000);
  int32_t micros_remainder = static_cast<int32_t>(now_micros % 1000000);
  constexpr size_t kTimeBufferSize = 30;
  char time_buffer[kTimeBufferSize];
  strftime(time_buffer, kTimeBufferSize, "%Y-%m-%d %H:%M:%S",
           localtime(&now_seconds));

  std::fprintf(FLAGS_logtostderr ? stderr : stdout, "%s.%06d: %c %s:%d] %s\n",
               time_buffer, micros_remainder, "IWEF"[severity_], file_name_,
               line_, str().c_str());
}

}  // namespace internal
}  // namespace xrtl
