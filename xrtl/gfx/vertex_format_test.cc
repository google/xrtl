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

#include "xrtl/gfx/vertex_format.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace {

// Tests the comparison operations on vertex formats.
TEST(VertexFormatTest, Comparisons) {
  EXPECT_FALSE(VertexFormats::kUndefined);
  EXPECT_TRUE(VertexFormats::kW2X10Y10Z10UNorm);
  EXPECT_EQ(VertexFormats::kUndefined, VertexFormats::kUndefined);
  EXPECT_NE(VertexFormats::kUndefined, VertexFormats::kW2X10Y10Z10UNorm);
  EXPECT_EQ(VertexFormats::kW2X10Y10Z10UNorm, VertexFormats::kW2X10Y10Z10UNorm);
}

// Tests some types and their math.
TEST(VertexFormatTest, Types) {
  VertexFormat format = VertexFormats::kW2X10Y10Z10UNorm;
  EXPECT_EQ(17, format.unique_id());
  EXPECT_EQ(ComponentFormat::kUNorm, format.component_format());
  EXPECT_EQ(4, format.component_count());
  EXPECT_EQ(10, format.component_bits_x());
  EXPECT_EQ(10, format.component_bits_y());
  EXPECT_EQ(10, format.component_bits_z());
  EXPECT_EQ(2, format.component_bits_w());
  EXPECT_EQ(4, format.data_size());
}

}  // namespace
}  // namespace gfx
}  // namespace xrtl
