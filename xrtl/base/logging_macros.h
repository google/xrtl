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

#ifndef XRTL_BASE_LOGGING_MACROS_H_
#define XRTL_BASE_LOGGING_MACROS_H_

#include <limits>
#include <sstream>
#include <string>

#include "xrtl/base/macros.h"

// Most of this code comes from the tensorflow header:
// https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/platform/default/logging.h
//
// The macros present here are a subset of those found in the Google logging
// code. Any additional macros required should be put in the XRTL-specific
// //xrtl/base/logging.h file.
//
// This file may go away in the future when a logging library is available
// to use in third_party.

namespace xrtl {

// Severity levels for LOG().
const int INFO = 0;            // base_logging::INFO;
const int WARNING = 1;         // base_logging::WARNING;
const int ERROR = 2;           // base_logging::ERROR;
const int FATAL = 3;           // base_logging::FATAL;
const int NUM_SEVERITIES = 4;  // base_logging::NUM_SEVERITIES;

namespace internal {

class LogMessage : public std::basic_ostringstream<char> {
 public:
  LogMessage(const char* file_name, int line, int severity);
  ~LogMessage();

  const char* file_name() const { return file_name_; }
  int line() const { return line_; }
  int severity() const { return severity_; }

  // Returns the minimum log level for VLOG statements.
  // E.g., if MinVLogLevel() is 2, then VLOG(2) statements will produce output,
  // but VLOG(3) will not. Defaults to 0.
  static int64_t MinVLogLevel();

 protected:
  // Defined per-platform to write out the message contents to a logging sink.
  // file_name/line/severity are optional and message.str() will return the
  // final log message contents.
  void EmitLogMessage();

 private:
  const char* file_name_;
  int line_;
  int severity_;
};

// LogMessageFatal ensures the process will exit in failure after
// logging this message.
class LogMessageFatal : public LogMessage {
 public:
  LogMessageFatal(const char* file, int line) XRTL_ATTRIBUTE_COLD;
  XRTL_ATTRIBUTE_NORETURN ~LogMessageFatal();
};

#define _XRTL_LOG_INFO \
  ::xrtl::internal::LogMessage(__FILE__, __LINE__, ::xrtl::INFO)
#define _XRTL_LOG_WARNING \
  ::xrtl::internal::LogMessage(__FILE__, __LINE__, ::xrtl::WARNING)
#define _XRTL_LOG_ERROR \
  ::xrtl::internal::LogMessage(__FILE__, __LINE__, ::xrtl::ERROR)
#define _XRTL_LOG_FATAL ::xrtl::internal::LogMessageFatal(__FILE__, __LINE__)

#define LOG(severity) _XRTL_LOG_##severity

#ifdef XRTL_CONFIG_LOGGING_VERBOSE
// Turn VLOG off at compile time when we want to reduce code size.
#define VLOG_IS_ON(lvl) ((lvl) <= 0)
#else
// Otherwise, Set XRTL_MIN_VLOG_LEVEL environment to update the minimum log
// level of VLOG at runtime.
#define VLOG_IS_ON(lvl) ((lvl) <= ::xrtl::internal::LogMessage::MinVLogLevel())
#endif  // XRTL_CONFIG_LOGGING_VERBOSE

#define VLOG(lvl)                          \
  if (XRTL_PREDICT_FALSE(VLOG_IS_ON(lvl))) \
  ::xrtl::internal::LogMessage(__FILE__, __LINE__, ::xrtl::INFO)

// CHECK dies with a fatal error if condition is not true.  It is *not*
// controlled by NDEBUG, so the check will be executed regardless of
// compilation mode.  Therefore, it is safe to do things like:
//    CHECK(fp->Write(x) == 4)
#define CHECK(condition)                \
  if (XRTL_PREDICT_FALSE(!(condition))) \
  LOG(FATAL) << "Check failed: " #condition " "

// Function is overloaded for integral types to allow static const
// integrals declared in classes and not defined to be used as arguments to
// CHECK* macros. It's not encouraged though.
template <typename T>
inline const T& GetReferenceableValue(const T& t) {
  return t;
}
inline char GetReferenceableValue(char t) { return t; }
inline unsigned char GetReferenceableValue(unsigned char t) { return t; }
inline signed char GetReferenceableValue(signed char t) { return t; }
inline short GetReferenceableValue(short t) { return t; }
inline unsigned short GetReferenceableValue(unsigned short t) { return t; }
inline int GetReferenceableValue(int t) { return t; }
inline unsigned int GetReferenceableValue(unsigned int t) { return t; }
inline long GetReferenceableValue(long t) { return t; }
inline unsigned long GetReferenceableValue(unsigned long t) { return t; }
inline long long GetReferenceableValue(long long t) { return t; }
inline unsigned long long GetReferenceableValue(unsigned long long t) {
  return t;
}

// This formats a value for a failing CHECK_XX statement.  Ordinarily,
// it uses the definition for operator<<, with a few special cases below.
template <typename T>
inline void MakeCheckOpValueString(std::ostream* os, const T& v) {
  (*os) << v;
}

// Overrides for char types provide readable values for unprintable
// characters.
template <>
void MakeCheckOpValueString(std::ostream* os, const char& v);
template <>
void MakeCheckOpValueString(std::ostream* os, const signed char& v);
template <>
void MakeCheckOpValueString(std::ostream* os, const unsigned char& v);

#if LANG_CXX11
// We need an explicit specialization for std::nullptr_t.
template <>
void MakeCheckOpValueString(std::ostream* os, const std::nullptr_t& p);
#endif

// A container for a string pointer which can be evaluated to a bool -
// true iff the pointer is non-NULL.
struct CheckOpString {
  CheckOpString(std::string* str) : str_(str) {}
  // No destructor: if str_ is non-NULL, we're about to LOG(FATAL),
  // so there's no point in cleaning up str_.
  operator bool() const { return XRTL_PREDICT_FALSE(str_ != NULL); }
  std::string* str_;
};

// Build the error message string. Specify no inlining for code size.
template <typename T1, typename T2>
std::string* MakeCheckOpString(const T1& v1, const T2& v2,
                               const char* exprtext) XRTL_ATTRIBUTE_NOINLINE;

// A helper class for formatting "expr (V1 vs. V2)" in a CHECK_XX
// statement.  See MakeCheckOpString for sample usage.  Other
// approaches were considered: use of a template method (e.g.,
// base::BuildCheckOpString(exprtext, base::Print<T1>, &v1,
// base::Print<T2>, &v2), however this approach has complications
// related to volatile arguments and function-pointer arguments).
class CheckOpMessageBuilder {
 public:
  // Inserts "exprtext" and " (" to the stream.
  explicit CheckOpMessageBuilder(const char* exprtext);
  // Deletes "stream_".
  ~CheckOpMessageBuilder();
  // For inserting the first variable.
  std::ostream* ForVar1() { return stream_; }
  // For inserting the second variable (adds an intermediate " vs. ").
  std::ostream* ForVar2();
  // Get the result (inserts the closing ")").
  std::string* NewString();

 private:
  std::ostringstream* stream_;
};

template <typename T1, typename T2>
std::string* MakeCheckOpString(const T1& v1, const T2& v2,
                               const char* exprtext) {
  CheckOpMessageBuilder comb(exprtext);
  MakeCheckOpValueString(comb.ForVar1(), v1);
  MakeCheckOpValueString(comb.ForVar2(), v2);
  return comb.NewString();
}

// Helper functions for CHECK_OP macro.
// The (int, int) specialization works around the issue that the compiler
// will not instantiate the template version of the function on values of
// unnamed enum type - see comment below.
// The (size_t, int) and (int, size_t) specialization are to handle unsigned
// comparison errors while still being thorough with the comparison.
#define _XRTL_DEFINE_CHECK_OP_IMPL(name, op)                             \
  template <typename T1, typename T2>                                    \
  inline std::string* name##Impl(const T1& v1, const T2& v2,             \
                                 const char* exprtext) {                 \
    if (XRTL_PREDICT_TRUE(v1 op v2))                                     \
      return NULL;                                                       \
    else                                                                 \
      return ::xrtl::internal::MakeCheckOpString(v1, v2, exprtext);      \
  }                                                                      \
  inline std::string* name##Impl(int v1, int v2, const char* exprtext) { \
    return name##Impl<int, int>(v1, v2, exprtext);                       \
  }                                                                      \
  inline std::string* name##Impl(const size_t v1, const int v2,          \
                                 const char* exprtext) {                 \
    if (XRTL_PREDICT_FALSE(v2 < 0)) {                                    \
      return ::xrtl::internal::MakeCheckOpString(v1, v2, exprtext);      \
    }                                                                    \
    const size_t uval = (size_t)((unsigned)v1);                          \
    return name##Impl<size_t, size_t>(uval, v2, exprtext);               \
  }                                                                      \
  inline std::string* name##Impl(const int v1, const size_t v2,          \
                                 const char* exprtext) {                 \
    if (XRTL_PREDICT_FALSE(v2 >= std::numeric_limits<int>::max())) {     \
      return ::xrtl::internal::MakeCheckOpString(v1, v2, exprtext);      \
    }                                                                    \
    const size_t uval = (size_t)((unsigned)v2);                          \
    return name##Impl<size_t, size_t>(v1, uval, exprtext);               \
  }

// We use the full name Check_EQ, Check_NE, etc. in case the file including
// base/logging.h provides its own #defines for the simpler names EQ, NE, etc.
// This happens if, for example, those are used as token names in a
// yacc grammar.
_XRTL_DEFINE_CHECK_OP_IMPL(Check_EQ,
                           ==)  // Compilation error with CHECK_EQ(NULL, x)?
_XRTL_DEFINE_CHECK_OP_IMPL(Check_NE, !=)  // Use CHECK(x == NULL) instead.
_XRTL_DEFINE_CHECK_OP_IMPL(Check_LE, <=)
_XRTL_DEFINE_CHECK_OP_IMPL(Check_LT, <)
_XRTL_DEFINE_CHECK_OP_IMPL(Check_GE, >=)
_XRTL_DEFINE_CHECK_OP_IMPL(Check_GT, >)
#undef _XRTL_DEFINE_CHECK_OP_IMPL

// In optimized mode, use CheckOpString to hint to compiler that
// the while condition is unlikely.
#define CHECK_OP_LOG(name, op, val1, val2)                      \
  while (::xrtl::internal::CheckOpString _result =              \
             ::xrtl::internal::name##Impl(                      \
                 ::xrtl::internal::GetReferenceableValue(val1), \
                 ::xrtl::internal::GetReferenceableValue(val2), \
                 #val1 " " #op " " #val2))                      \
  ::xrtl::internal::LogMessageFatal(__FILE__, __LINE__) << *(_result.str_)

#define CHECK_OP(name, op, val1, val2) CHECK_OP_LOG(name, op, val1, val2)

// CHECK_EQ/NE/...
#define CHECK_EQ(val1, val2) CHECK_OP(Check_EQ, ==, val1, val2)
#define CHECK_NE(val1, val2) CHECK_OP(Check_NE, !=, val1, val2)
#define CHECK_LE(val1, val2) CHECK_OP(Check_LE, <=, val1, val2)
#define CHECK_LT(val1, val2) CHECK_OP(Check_LT, <, val1, val2)
#define CHECK_GE(val1, val2) CHECK_OP(Check_GE, >=, val1, val2)
#define CHECK_GT(val1, val2) CHECK_OP(Check_GT, >, val1, val2)
#define CHECK_NOTNULL(val)                           \
  ::xrtl::internal::CheckNotNull(__FILE__, __LINE__, \
                                 "'" #val "' Must be non NULL", (val))

#ifndef NDEBUG
// DCHECK_EQ/NE/...
#define DCHECK(condition) CHECK(condition)
#define DCHECK_EQ(val1, val2) CHECK_EQ(val1, val2)
#define DCHECK_NE(val1, val2) CHECK_NE(val1, val2)
#define DCHECK_LE(val1, val2) CHECK_LE(val1, val2)
#define DCHECK_LT(val1, val2) CHECK_LT(val1, val2)
#define DCHECK_GE(val1, val2) CHECK_GE(val1, val2)
#define DCHECK_GT(val1, val2) CHECK_GT(val1, val2)

#else

#define DCHECK(condition) \
  while (false && (condition)) LOG(FATAL)

// NDEBUG is defined, so DCHECK_EQ(x, y) and so on do nothing.
// However, we still want the compiler to parse x and y, because
// we don't want to lose potentially useful errors and warnings.
// _DCHECK_NOP is a helper, and should not be used outside of this file.
#define _XRTL_DCHECK_NOP(x, y) \
  while (false && ((void)(x), (void)(y), 0)) LOG(FATAL)

#define DCHECK_EQ(x, y) _XRTL_DCHECK_NOP(x, y)
#define DCHECK_NE(x, y) _XRTL_DCHECK_NOP(x, y)
#define DCHECK_LE(x, y) _XRTL_DCHECK_NOP(x, y)
#define DCHECK_LT(x, y) _XRTL_DCHECK_NOP(x, y)
#define DCHECK_GE(x, y) _XRTL_DCHECK_NOP(x, y)
#define DCHECK_GT(x, y) _XRTL_DCHECK_NOP(x, y)

#endif  // !NDEBUG

template <typename T>
T&& CheckNotNull(const char* file, int line, const char* exprtext, T&& t) {
  if (t == nullptr) {
    LogMessageFatal(file, line) << std::string(exprtext);
  }
  return std::forward<T>(t);
}

}  // namespace internal
}  // namespace xrtl

#endif  // XRTL_BASE_LOGGING_MACROS_H_
