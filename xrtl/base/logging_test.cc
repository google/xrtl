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

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

// NOTE: we just test that all the macros are defined, code compiles,
//       and there are no runtime crashes.

TEST(LoggingTest, Logs) {
  LOG(INFO) << "Hello";
  LOG(INFO) << "Another log message";
  LOG(ERROR) << "Error message";
  VLOG(1) << "A VLOG message";
  VLOG(2) << "A higher VLOG message";
}

TEST(LoggingTest, Checks) {
  CHECK(true);
  CHECK(7 > 5);
  std::string a("abc");
  std::string b("xyz");
  CHECK_EQ(a, a);
  CHECK_NE(a, b);
  CHECK_EQ(3, 3);
  CHECK_NE(4, 3);
  CHECK_GT(4, 3);
  CHECK_GE(3, 3);
  CHECK_LT(2, 3);
  CHECK_LE(2, 3);

  DCHECK(true);
  DCHECK(7 > 5);
  DCHECK_EQ(a, a);
  DCHECK_NE(a, b);
  DCHECK_EQ(3, 3);
  DCHECK_NE(4, 3);
  DCHECK_GT(4, 3);
  DCHECK_GE(3, 3);
  DCHECK_LT(2, 3);
  DCHECK_LE(2, 3);
}

TEST(LoggingTest, FailedChecks) {
  const char* p_const = "hello there";
  CHECK_NOTNULL(p_const);

  char mybuf[10];
  char* p_non_const = mybuf;
  CHECK_NOTNULL(p_non_const);

// Disabled due to some weird gtest issues.
// TODO(benvanik): figure out why the assert value does not match.
#if 0
  std::string a("abc");
  std::string b("xyz");

  ASSERT_DEATH(CHECK(false), "false");
  ASSERT_DEATH(CHECK(9 < 7), "9 < 7");
  ASSERT_DEATH(CHECK_EQ(a, b), "a == b");
  ASSERT_DEATH(CHECK_EQ(3, 4), "3 == 4");
  ASSERT_DEATH(CHECK_NE(3, 3), "3 != 3");
  ASSERT_DEATH(CHECK_GT(2, 3), "2 > 3");
  ASSERT_DEATH(CHECK_GE(2, 3), "2 >= 3");
  ASSERT_DEATH(CHECK_LT(3, 2), "3 < 2");
  ASSERT_DEATH(CHECK_LE(3, 2), "3 <= 2");
  ASSERT_DEATH(CHECK(false), "false");
  char* p_null = nullptr;
  ASSERT_DEATH(printf("%s", CHECK_NOTNULL(p_null)), "Must be non NULL");
  const char* p_null_const = nullptr;
  ASSERT_DEATH(printf("%s", CHECK_NOTNULL(p_null_const)), "Must be non NULL");
#ifndef NDEBUG
  ASSERT_DEATH(DCHECK(9 < 7), "9 < 7");
  ASSERT_DEATH(DCHECK(9 < 7), "9 < 7");
  ASSERT_DEATH(DCHECK_EQ(a, b), "a == b");
  ASSERT_DEATH(DCHECK_EQ(3, 4), "3 == 4");
  ASSERT_DEATH(DCHECK_NE(3, 3), "3 != 3");
  ASSERT_DEATH(DCHECK_GT(2, 3), "2 > 3");
  ASSERT_DEATH(DCHECK_GE(2, 3), "2 >= 3");
  ASSERT_DEATH(DCHECK_LT(3, 2), "3 < 2");
  ASSERT_DEATH(DCHECK_LE(3, 2), "3 <= 2");
#endif  // !NDEBUG

#endif  // 0
}

TEST(LoggingTest, LogString) {
  LogString(__FILE__, __LINE__, 0, "Hello there");
  LogString(__FILE__, __LINE__, 0, std::string("Hello std"));
  LogString(__FILE__, __LINE__, 0, "Hello there", 3);
}

TEST(LoggingTest, LogStringFormat) {
  LogStringFormat(__FILE__, __LINE__, 0, "Hello there");
  LogStringFormat(__FILE__, __LINE__, 0, "Hello %d", 5);
}

}  // namespace
}  // namespace xrtl
