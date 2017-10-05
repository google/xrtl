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

#include "xrtl/testing/diffing/text_differ.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace testing {
namespace diffing {
namespace {

// Tests a variety of normal string types.
TEST(TextDifferTest, SimpleComparisons) {
  EXPECT_TRUE(TextDiffer::CompareStrings(nullptr, nullptr, {}));
  EXPECT_FALSE(TextDiffer::CompareStrings("foo", nullptr, {}));
  EXPECT_TRUE(TextDiffer::CompareStrings("", "", {}));
  EXPECT_FALSE(TextDiffer::CompareStrings("foo", "", {}));
  EXPECT_TRUE(TextDiffer::CompareStrings("a", "a", {}));
  EXPECT_FALSE(TextDiffer::CompareStrings("a", "b", {}));
  EXPECT_FALSE(TextDiffer::CompareStrings("a", "aa", {}));
}

// Tests binary data (including NUL and such) types.
TEST(TextDifferTest, BinaryComparisons) {
  EXPECT_TRUE(TextDiffer::CompareStrings(absl::string_view("a\0b", 3),
                                         absl::string_view("a\0b", 3), {}));
  EXPECT_FALSE(TextDiffer::CompareStrings(absl::string_view("a\0c", 3),
                                          absl::string_view("a\0b", 3), {}));
}

}  // namespace
}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
