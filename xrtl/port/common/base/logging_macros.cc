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

#include "xrtl/port/common/base/logging_macros.h"

#include <cstdio>
#include <cstdlib>

#include "xrtl/base/flags.h"
#include "xrtl/base/logging.h"

DEFINE_int32(minloglevel, 0, "Minimum logging level. 0 = INFO and above.");
DEFINE_int32(v, 0, "Verbosity level maximum. 1 = VLOG(0-1), 2 = VLOG(0-2).");

namespace xrtl {
namespace internal {

LogMessage::LogMessage(const char* file_name, int line, int severity)
    : file_name_(file_name), line_(line), severity_(severity) {}

namespace {

// Parse log level (int64_t) from environment variable (char*).
// Returns true if the value was present and parsed successfully.
bool LogLevelStrToInt(const char* xrtl_env_var_val, int64_t* out_level) {
  *out_level = 0;
  if (xrtl_env_var_val == nullptr) {
    return false;
  }

  // Ideally we would use env_var / safe_strto64, but it is
  // hard to use here without pulling in a lot of dependencies,
  // so we use std:istringstream instead.
  std::string min_log_level(xrtl_env_var_val);
  std::istringstream ss(min_log_level);
  int64_t level;
  if (!(ss >> level)) {
    // Invalid vlog level setting, set level to default (0).
    return false;
  }

  *out_level = level;
  return true;
}

int64_t MinLogLevelFromEnv() {
  const char* xrtl_env_var_val = getenv("XRTL_MIN_LOG_LEVEL");
  int64_t level = 0;
  if (LogLevelStrToInt(xrtl_env_var_val, &level)) {
    return level;
  }
  return FLAGS_minloglevel;
}

int64_t MinVLogLevelFromEnv() {
  const char* xrtl_env_var_val = getenv("XRTL_MIN_VLOG_LEVEL");
  int64_t level = 0;
  if (LogLevelStrToInt(xrtl_env_var_val, &level)) {
    return level;
  }
  return FLAGS_v;
}

}  // namespace

LogMessage::~LogMessage() {
  // Read the min log level once during the first call to logging.
  static int64_t min_log_level = MinLogLevelFromEnv();
  if (XRTL_PREDICT_TRUE(severity_ >= min_log_level)) {
    EmitLogMessage();
  }
}

int64_t LogMessage::MinVLogLevel() {
  static int64_t min_vlog_level = MinVLogLevelFromEnv();
  return min_vlog_level;
}

LogMessageFatal::LogMessageFatal(const char* file, int line)
    : LogMessage(file, line, FATAL) {}

LogMessageFatal::~LogMessageFatal() {
  EmitLogMessage();

#if defined(XRTL_COMPILER_MSVC)
// MSVC warns about the abort not returning.
#pragma warning(suppress : 4722)
#endif  // XRTL_COMPILER_MSVC

  // abort() ensures we don't return (we promised we would not via
  // ATTRIBUTE_NORETURN).
  abort();
}

template <>
void MakeCheckOpValueString(std::ostream* os, const char& v) {
  if (v >= 32 && v <= 126) {
    (*os) << "'" << v << "'";
  } else {
    (*os) << "char value " << static_cast<int16_t>(v);
  }
}

template <>
void MakeCheckOpValueString(std::ostream* os, const int8_t& v) {
  if (v >= 32 && v <= 126) {
    (*os) << "'" << v << "'";
  } else {
    (*os) << "signed char value " << static_cast<int16_t>(v);
  }
}

template <>
void MakeCheckOpValueString(std::ostream* os, const uint8_t& v) {
  if (v >= 32 && v <= 126) {
    (*os) << "'" << v << "'";
  } else {
    (*os) << "unsigned char value " << static_cast<uint16_t>(v);
  }
}

#if LANG_CXX11
template <>
void MakeCheckOpValueString(std::ostream* os, const std::nullptr_t& p) {
  (*os) << "nullptr";
}
#endif

CheckOpMessageBuilder::CheckOpMessageBuilder(const char* exprtext)
    : stream_(new std::ostringstream) {
  *stream_ << "Check failed: " << exprtext << " (";
}

CheckOpMessageBuilder::~CheckOpMessageBuilder() { delete stream_; }

std::ostream* CheckOpMessageBuilder::ForVar2() {
  *stream_ << " vs. ";
  return stream_;
}

std::string* CheckOpMessageBuilder::NewString() {
  *stream_ << ")";
  return new std::string(stream_->str());
}

}  // namespace internal

void LogString(const char* file_name, int line, int severity,
               const char* message, size_t message_length) {
  internal::LogMessage(file_name, line, severity) << message;
}

}  // namespace xrtl
