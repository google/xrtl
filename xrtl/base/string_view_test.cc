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

#include "xrtl/base/string_view.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

constexpr auto npos = StringView::npos;

class StringViewTest : public ::testing::Test {};

TEST_F(StringViewTest, Construction) {
  // Default string view is empty string.
  StringView empty_str;
  EXPECT_EQ(0, empty_str.size());
  EXPECT_TRUE(empty_str.empty());
  EXPECT_NE(nullptr, empty_str.data());
  EXPECT_EQ("", std::string(empty_str));

  // std::string wrapping should not copy the data.
  std::string hello_str = "hello";
  StringView std_str(hello_str);
  EXPECT_EQ(5, std_str.size());
  EXPECT_FALSE(std_str.empty());
  EXPECT_EQ(hello_str.data(), std_str.data());
  EXPECT_EQ(hello_str, std::string(std_str));

  // char* without size should calculate the size properly.
  StringView no_size_empty_str("");
  EXPECT_EQ(0, no_size_empty_str.size());
  EXPECT_EQ("", std::string(no_size_empty_str));
  StringView no_data_empty_str(nullptr);
  EXPECT_EQ(0, no_data_empty_str.size());
  EXPECT_EQ("", std::string(no_data_empty_str));
  StringView no_size_str("hello");
  EXPECT_EQ(5, no_size_str.size());
  EXPECT_EQ("hello", std::string(no_size_str));
}

TEST_F(StringViewTest, DataAccess) {
  StringView empty_str("");
  EXPECT_EQ(0, empty_str.size());
  EXPECT_EQ(empty_str.data(), empty_str.begin());
  EXPECT_EQ(empty_str.begin(), empty_str.end());

  StringView one_char_str("a");
  EXPECT_EQ(1, one_char_str.size());
  EXPECT_EQ(one_char_str.data(), one_char_str.begin());
  EXPECT_EQ(one_char_str.data() + 1, one_char_str.end());
  EXPECT_EQ('a', one_char_str[0]);

  StringView long_str("abcd");
  EXPECT_EQ(4, long_str.size());
  EXPECT_EQ(long_str.data(), long_str.begin());
  EXPECT_EQ(long_str.data() + 4, long_str.end());
  EXPECT_EQ('a', long_str[0]);
  EXPECT_EQ('b', long_str[1]);
  EXPECT_EQ('c', long_str[2]);
  EXPECT_EQ('d', long_str[3]);
  EXPECT_EQ("abcd", long_str.data());
  EXPECT_EQ("abcd", long_str.begin());
}

TEST_F(StringViewTest, Substr) {
  StringView empty_str("");
  auto e0 = empty_str.substr(0, npos);
  EXPECT_EQ(empty_str.size(), e0.size());
  EXPECT_EQ(empty_str.data(), e0.data());
  auto e1 = empty_str.substr(0, 0);
  EXPECT_EQ(empty_str.size(), e1.size());
  EXPECT_EQ(empty_str.data(), e1.data());
  auto e2 = empty_str.substr(0, 1);
  EXPECT_EQ(empty_str.size(), e2.size());
  EXPECT_EQ(empty_str.data(), e2.data());

  StringView one_char_str("a");
  auto o0 = one_char_str.substr(0, npos);
  EXPECT_EQ(one_char_str.size(), o0.size());
  EXPECT_EQ(one_char_str.data(), o0.data());
  EXPECT_EQ('a', one_char_str[0]);
  auto o1 = one_char_str.substr(0, 0);
  EXPECT_EQ(0, o1.size());
  EXPECT_EQ(one_char_str.data(), o1.data());
  auto o2 = one_char_str.substr(0, 1);
  EXPECT_EQ(1, o2.size());
  EXPECT_EQ(one_char_str.data(), o2.data());
  EXPECT_EQ('a', one_char_str[0]);

  StringView long_str("abcd");
  auto l0 = long_str.substr(0, npos);
  EXPECT_EQ(long_str.size(), l0.size());
  EXPECT_EQ(long_str.data(), l0.data());
  auto l1 = long_str.substr(0, 0);
  EXPECT_EQ(0, l1.size());
  EXPECT_EQ(long_str.data(), l1.data());
  auto l2 = long_str.substr(0, 1);
  EXPECT_EQ(1, l2.size());
  EXPECT_EQ(long_str.data(), l2.data());
  EXPECT_EQ('a', long_str[0]);
  auto l3 = long_str.substr(1, npos);
  EXPECT_EQ(3, l3.size());
  EXPECT_EQ(long_str.data() + 1, l3.data());
  EXPECT_EQ("bcd", l3);
  auto l4 = long_str.substr(1, 1);
  EXPECT_EQ(1, l4.size());
  EXPECT_EQ(long_str.data() + 1, l4.data());
  EXPECT_EQ("b", l4);
  auto l5 = long_str.substr(1, 100);
  EXPECT_EQ(3, l5.size());
  EXPECT_EQ(long_str.data() + 1, l5.data());
  EXPECT_EQ("bcd", l5);
  auto l6 = long_str.substr(3, npos);
  EXPECT_EQ(1, l6.size());
  EXPECT_EQ(long_str.data() + 3, l6.data());
  EXPECT_EQ("d", l6);
  auto l7 = long_str.substr(3, 1);
  EXPECT_EQ(1, l7.size());
  EXPECT_EQ(long_str.data() + 3, l7.data());
  EXPECT_EQ("d", l7);
  auto l8 = long_str.substr(4, npos);
  EXPECT_EQ(0, l8.size());
  EXPECT_EQ("", l8);
  auto l9 = long_str.substr(4, 1);
  EXPECT_EQ(0, l9.size());
  EXPECT_EQ("", l9);
  auto l10 = long_str.substr(4, 0);
  EXPECT_EQ(0, l10.size());
  EXPECT_EQ("", l10);
}

TEST_F(StringViewTest, FindFirstOfString) {
  StringView empty_str("");
  EXPECT_EQ(0, empty_str.find_first_of("\0", 0));
  EXPECT_EQ(npos, empty_str.find_first_of(StringView("a"), 0));
  EXPECT_EQ(npos, empty_str.find_first_of(StringView("a"), 1));
  EXPECT_EQ(npos, empty_str.find_first_of(StringView("a"), npos));

  StringView one_char_str("a");
  EXPECT_EQ(0, one_char_str.find_first_of("\0", 0));
  EXPECT_EQ(0, one_char_str.find_first_of("a", 0));
  EXPECT_EQ(npos, one_char_str.find_first_of("a", 1));
  EXPECT_EQ(npos, one_char_str.find_first_of("a", npos));
  EXPECT_EQ(npos, one_char_str.find_first_of("abc", 0));
  EXPECT_EQ(npos, one_char_str.find_first_of("abc", 1));
  EXPECT_EQ(npos, one_char_str.find_first_of("abc", npos));

  StringView long_str("abcd");
  EXPECT_EQ(1, long_str.find_first_of("bc", 0));
  EXPECT_EQ(1, long_str.find_first_of("bc", 1));
  EXPECT_EQ(npos, long_str.find_first_of("bc", 2));
  EXPECT_EQ(npos, long_str.find_first_of("bc", 4));
  EXPECT_EQ(npos, long_str.find_first_of("bc", npos));
  EXPECT_EQ(npos, long_str.find_first_of("abcxx", 0));
  EXPECT_EQ(npos, long_str.find_first_of("abcxx", 1));
  EXPECT_EQ(npos, long_str.find_first_of("abcxx", npos));
  EXPECT_EQ(npos, long_str.find_first_of("xbcd", 0));
  EXPECT_EQ(npos, long_str.find_first_of("xbcd", 1));
  EXPECT_EQ(npos, long_str.find_first_of("xbcd", npos));

  StringView repeated_str("ababab");
  EXPECT_EQ(0, repeated_str.find_first_of("ab", 0));
  EXPECT_EQ(2, repeated_str.find_first_of("ab", 1));
  EXPECT_EQ(2, repeated_str.find_first_of("ab", 2));
  EXPECT_EQ(npos, repeated_str.find_first_of("ab", 5));
  EXPECT_EQ(npos, repeated_str.find_first_of("ab", 6));
  EXPECT_EQ(npos, repeated_str.find_first_of("ab", npos));
  EXPECT_EQ(npos, repeated_str.find_first_of("ac", 0));
  EXPECT_EQ(npos, repeated_str.find_first_of("ac", 1));
  EXPECT_EQ(npos, repeated_str.find_first_of("ac", npos));
  EXPECT_EQ(npos, repeated_str.find_first_of("cd", 0));
  EXPECT_EQ(npos, repeated_str.find_first_of("cd", 5));
  EXPECT_EQ(npos, repeated_str.find_first_of("cd", 6));
  EXPECT_EQ(npos, repeated_str.find_first_of("cd", npos));
}

TEST_F(StringViewTest, FindFirstOfChar) {
  StringView empty_str("");
  EXPECT_EQ(npos, empty_str.find_first_of('a', 0));
  EXPECT_EQ(npos, empty_str.find_first_of('a', 1));
  EXPECT_EQ(npos, empty_str.find_first_of('a', npos));

  StringView one_char_str("a");
  EXPECT_EQ(0, one_char_str.find_first_of('a', 0));
  EXPECT_EQ(npos, one_char_str.find_first_of('a', 1));
  EXPECT_EQ(npos, one_char_str.find_first_of('a', npos));
  EXPECT_EQ(npos, one_char_str.find_first_of('b', 0));
  EXPECT_EQ(npos, one_char_str.find_first_of('b', 1));
  EXPECT_EQ(npos, one_char_str.find_first_of('b', npos));

  StringView long_str("abcd");
  EXPECT_EQ(1, long_str.find_first_of('b', 0));
  EXPECT_EQ(1, long_str.find_first_of('b', 1));
  EXPECT_EQ(npos, long_str.find_first_of('b', 4));
  EXPECT_EQ(npos, long_str.find_first_of('b', npos));
  EXPECT_EQ(npos, long_str.find_first_of('x', 0));
  EXPECT_EQ(npos, long_str.find_first_of('x', 1));
  EXPECT_EQ(npos, long_str.find_first_of('x', npos));

  StringView repeated_str("aaaa");
  EXPECT_EQ(0, repeated_str.find_first_of('a', 0));
  EXPECT_EQ(1, repeated_str.find_first_of('a', 1));
  EXPECT_EQ(2, repeated_str.find_first_of('a', 2));
  EXPECT_EQ(3, repeated_str.find_first_of('a', 3));
  EXPECT_EQ(npos, repeated_str.find_first_of('a', 4));
  EXPECT_EQ(npos, repeated_str.find_first_of('a', npos));
  EXPECT_EQ(npos, repeated_str.find_first_of('c', 0));
  EXPECT_EQ(npos, repeated_str.find_first_of('c', 5));
  EXPECT_EQ(npos, repeated_str.find_first_of('c', 6));
  EXPECT_EQ(npos, repeated_str.find_first_of('c', npos));
}

TEST_F(StringViewTest, FindLastOfChar) {
  StringView empty_str("");
  EXPECT_EQ(npos, empty_str.find_last_of('a', 0));
  EXPECT_EQ(npos, empty_str.find_last_of('a', 1));
  EXPECT_EQ(npos, empty_str.find_last_of('a', npos));

  StringView one_char_str("a");
  EXPECT_EQ(0, one_char_str.find_last_of('a', 0));
  EXPECT_EQ(0, one_char_str.find_last_of('a', 1));
  EXPECT_EQ(0, one_char_str.find_last_of('a', npos));
  EXPECT_EQ(npos, one_char_str.find_last_of('b', 0));
  EXPECT_EQ(npos, one_char_str.find_last_of('b', 1));
  EXPECT_EQ(npos, one_char_str.find_last_of('b', npos));

  StringView long_str("abcd");
  EXPECT_EQ(npos, long_str.find_last_of('b', 0));
  EXPECT_EQ(1, long_str.find_last_of('b', 1));
  EXPECT_EQ(1, long_str.find_last_of('b', 2));
  EXPECT_EQ(1, long_str.find_last_of('b', 4));
  EXPECT_EQ(1, long_str.find_last_of('b', npos));
  EXPECT_EQ(npos, long_str.find_last_of('x', 0));
  EXPECT_EQ(npos, long_str.find_last_of('x', 1));
  EXPECT_EQ(npos, long_str.find_last_of('x', npos));

  StringView repeated_str("aaaa");
  EXPECT_EQ(0, repeated_str.find_last_of('a', 0));
  EXPECT_EQ(1, repeated_str.find_last_of('a', 1));
  EXPECT_EQ(2, repeated_str.find_last_of('a', 2));
  EXPECT_EQ(3, repeated_str.find_last_of('a', 3));
  EXPECT_EQ(3, repeated_str.find_last_of('a', 4));
  EXPECT_EQ(3, repeated_str.find_last_of('a', npos));
  EXPECT_EQ(npos, repeated_str.find_last_of('c', 0));
  EXPECT_EQ(npos, repeated_str.find_last_of('c', 5));
  EXPECT_EQ(npos, repeated_str.find_last_of('c', 6));
  EXPECT_EQ(npos, repeated_str.find_last_of('c', npos));
}

TEST_F(StringViewTest, Comparison) {
  EXPECT_EQ(0, StringView("").compare(""));
  EXPECT_NE(0, StringView("").compare("x"));
  EXPECT_NE(0, StringView("x").compare(""));
  EXPECT_EQ(0, StringView("x").compare("x"));
  EXPECT_NE(0, StringView("x").compare("X"));
  EXPECT_NE(0, StringView("X").compare("x"));
  EXPECT_EQ(0, StringView("X").compare("X"));
  EXPECT_LT(0, StringView("x").compare("y"));
  EXPECT_GT(0, StringView("x").compare("abc"));
  EXPECT_LT(0, StringView("abc").compare("x"));
  EXPECT_EQ(0, StringView("abc").compare("abc"));
  EXPECT_LT(0, StringView("abc").compare("def"));
  EXPECT_LT(0, StringView("1").compare("2"));
  EXPECT_GT(0, StringView("2").compare("1"));
  EXPECT_EQ(0, StringView("1").compare("1"));

  EXPECT_TRUE(StringView("").equals(""));
  EXPECT_FALSE(StringView("").equals("x"));
  EXPECT_FALSE(StringView("x").equals(""));
  EXPECT_TRUE(StringView("x").equals("x"));
  EXPECT_FALSE(StringView("x").equals("X"));
  EXPECT_FALSE(StringView("X").equals("x"));
  EXPECT_TRUE(StringView("X").equals("X"));
  char xs0[] = {'x', 0, 1};
  char xs1[] = {'x', 0, 2};
  EXPECT_NE(xs0, xs1);
  EXPECT_TRUE(StringView(xs0).equals(xs1));
  char null0[] = {0, 1};
  char null1[] = {0, 2};
  EXPECT_NE(null0, null1);
  EXPECT_TRUE(StringView(null0).equals(null1));
  EXPECT_FALSE(StringView("x").equals("abc"));
  EXPECT_FALSE(StringView("abc").equals("x"));
  EXPECT_TRUE(StringView("abc").equals("abc"));
  EXPECT_FALSE(StringView("abc").equals("def"));
}

}  // namespace
}  // namespace xrtl
