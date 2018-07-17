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

#include "xrtl/base/proto_util.h"

#include "xrtl/base/protobuf.h"
#include "xrtl/base/testdata/proto_util_test.pb.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

TEST(ProtoUtilTest, TextPb) {
  const char* kTextPb = "int32_value: 5\nstring_value: \"foo\"\n";

  // Deserialize from string.
  xrtl::testdata::SimpleMessage message;
  EXPECT_TRUE(xrtl::protobuf::TextFormat::ParseFromString(kTextPb, &message));
  EXPECT_EQ(5, message.int32_value());
  EXPECT_EQ("foo", message.string_value());

  // Serialize to string.
  EXPECT_EQ(kTextPb, message.DebugString());
}

}  // namespace
}  // namespace xrtl
