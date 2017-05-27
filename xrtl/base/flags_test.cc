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

#include "xrtl/base/flags.h"

#include "xrtl/testing/gtest.h"

DEFINE_bool(cmd_line_flag, false, "cmd_line_flag");

DEFINE_bool(bool_flag, true, "bool");
DEFINE_int32(int32_flag, 123, "int32");
DEFINE_string(string_flag, "hello", "string");
DEFINE_double(double_flag, 1.23, "double");

namespace xrtl {
namespace flags {
namespace {

TEST(FlagsTest, Initialization) { SetVersionString("1.2"); }

TEST(FlagsTest, CommandLines) {
  FLAGS_cmd_line_flag = false;
  EXPECT_EQ(false, FLAGS_cmd_line_flag);
  std::vector<const char*> args = {
      "test", "--cmd_line_flag=true", "other",
  };
  int argc = static_cast<int>(args.size());
  char** argv = const_cast<char**>(args.data());
  uint32_t index = ParseCommandLineFlags(&argc, &argv, true);
  EXPECT_EQ(2, argc);
  EXPECT_EQ(1, index);
  EXPECT_EQ("other", argv[1]);
  EXPECT_EQ(true, FLAGS_cmd_line_flag);
}

TEST(FlagsTest, GetAllFlagNames) {
  auto flag_names = GetAllFlagNames();
  bool has_this_test_flags = false;
  for (const auto& flag_name : flag_names) {
    if (flag_name == "cmd_line_flag") {
      has_this_test_flags = true;
      break;
    }
  }
  EXPECT_TRUE(has_this_test_flags);
}

TEST(FlagsTest, DynamicFlagValues) {
  std::string missing_value = "dummy";
  EXPECT_FALSE(GetFlagValue("missing_flag", &missing_value));
  EXPECT_TRUE(missing_value.empty());
  EXPECT_EQ("missing", GetFlagValue("missing_flag", "missing"));

  FLAGS_bool_flag = true;
  std::string bool_value = "dummy";
  EXPECT_TRUE(GetFlagValue("bool_flag", &bool_value));
  EXPECT_EQ("true", bool_value);
  EXPECT_EQ("true", GetFlagValue("bool_flag", "false"));

  FLAGS_bool_flag = true;
  EXPECT_EQ("true", GetFlagValue("bool_flag", ""));
  SetFlagValue("bool_flag", "false");
  EXPECT_EQ("false", GetFlagValue("bool_flag", ""));

  FLAGS_int32_flag = 123;
  EXPECT_EQ("123", GetFlagValue("int32_flag", ""));
  SetFlagValue("int32_flag", "456");
  EXPECT_EQ("456", GetFlagValue("int32_flag", ""));

  FLAGS_string_flag = "hello";
  EXPECT_EQ("hello", GetFlagValue("string_flag", ""));
  SetFlagValue("string_flag", "world");
  EXPECT_EQ("world", GetFlagValue("string_flag", ""));

  FLAGS_double_flag = 1.23;
  EXPECT_EQ("1.23", GetFlagValue("double_flag", ""));
  SetFlagValue("double_flag", "4.0");
  EXPECT_EQ("4", GetFlagValue("double_flag", ""));
}

}  // namespace
}  // namespace flags
}  // namespace xrtl
