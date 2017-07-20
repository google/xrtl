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

#include <android/log.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "xrtl/base/logging.h"
#include "xrtl/port/common/base/logging_macros.h"

namespace xrtl {

void FlushLog() { std::fflush(stderr); }

namespace internal {

void LogMessage::EmitLogMessage() {
  int android_log_level;
  switch (severity()) {
    case INFO:
      android_log_level = ANDROID_LOG_INFO;
      break;
    case WARNING:
      android_log_level = ANDROID_LOG_WARN;
      break;
    case ERROR:
      android_log_level = ANDROID_LOG_ERROR;
      break;
    case FATAL:
      android_log_level = ANDROID_LOG_FATAL;
      break;
    default:
      if (severity_ < INFO) {
        android_log_level = ANDROID_LOG_VERBOSE;
      } else {
        android_log_level = ANDROID_LOG_ERROR;
      }
      break;
  }

  const char* const partial_name = strrchr(file_name(), '/');
  __android_log_print(android_log_level, "xrtl", "%s:%d %s",
                      partial_name ? partial_name + 1 : file_name(), line(),
                      str().c_str());

  // Also log to stderr (for standalone Android apps).
  std::fprintf(stderr, "xrtl %s:%d %s\n",
               partial_name ? partial_name + 1 : file_name(), line(),
               str().c_str());

  // Android logging at level FATAL does not terminate execution, so abort()
  // is still required to stop the program.
  if (severity() == FATAL) {
    abort();
  }
}

}  // namespace internal
}  // namespace xrtl
