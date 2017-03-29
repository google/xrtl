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

#ifndef XRTL_BASE_LOGGING_H_
#define XRTL_BASE_LOGGING_H_

#include <cstring>
#include <string>

// Logging macros live in their own file so that we can use external versions
// as required.
//
// LOG(severity) << ...;
//   Logs a message at the given severity.
//   Severity:
//     INFO    Logs information text.
//     WARNING Logs a warning.
//     ERROR   Logs an error.
//     FATAL   Logs an error and exit(1).
//
// VLOG(level) << ...;
//   Logs a verbose message at the given verbosity level.
//
// CHECK(condition) << ...;
//   Runtime asserts that the given condition is true even in release builds.
//   It's recommended that DCHECK is used instead as too many CHECKs
//   can impact performance.
//
// CHECK_EQ|NE|LT|GT|LE|GE(val1, val2) << ...;
//   Runtime assert the specified operation with the given values.
//
// value = CHECK_NOTNULL(value);
//   Runtime assert the given value is not null and return the value.
//   This can be used in initializers as the value is returned.
//
// DCHECK(condition) << ...;
//   Runtime asserts that the given condition is true only in non-opt
//   builds.
//
// DCHECK_EQ|NE|LT|GT|LE|GE(val1, val2) << ...;
//   Runtime assert the specified operation with the given values in non-opt
//   builds.

#ifdef XRTL_CONFIG_GOOGLE_INTERNAL
#include "xrtl/port/google/base/logging_forward.h"
#else
#include "xrtl/port/common/base/logging_macros.h"
#endif  // XRTL_CONFIG_GOOGLE_INTERNAL

namespace xrtl {

// Flushes log contents immediately.
void FlushLog();

// Emits a message to the log at the specified severity.
// The message is logged as if from `LOG(severity) << message;`.
inline void LogString(const char* file_name, int line, int severity,
                      const std::string& message) {
  LogString(file_name, line, severity, message.c_str(), message.size());
}
inline void LogString(const char* file_name, int line, int severity,
                      const char* message) {
  LogString(file_name, line, severity, message, std::strlen(message));
}

// Emits a message to the log with printf-style formatting.
// This uses a thread-local buffer and is thread-safe.
void LogStringFormat(const char* file_name, int line, int severity,
                     const char* fmt, ...);

}  // namespace xrtl

#endif  // XRTL_BASE_LOGGING_H_
