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

#include "xrtl/gfx/pixel_format.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace {

// Tests the comparison operations on pixel formats.
TEST(PixelFormatTest, Comparisons) {
  EXPECT_FALSE(PixelFormats::kUndefined);
  EXPECT_TRUE(PixelFormats::kA2B10G10R10UNorm);
  EXPECT_EQ(PixelFormats::kUndefined, PixelFormats::kUndefined);
  EXPECT_NE(PixelFormats::kUndefined, PixelFormats::kA2B10G10R10UNorm);
}

// Tests the basic usage of PixelFormatTable.
TEST(PixelFormatTest, PixelFormatTable) {
  struct MyEntry {
    int index;
    MyEntry(const MyEntry& other) = delete;  // Ensure we don't copy.
  };
  static const MyEntry kTable[] = {
      {0}, {1}, {2}, {3}, {4}, {5},
  };
  PixelFormatTable<MyEntry, PixelFormats::kEtc2R8G8B8UNorm,
                   PixelFormats::kEtc2R8G8B8A8Srgb>
      table(kTable);
  EXPECT_EQ(6, table.range());
  EXPECT_EQ(6, table.size());
  EXPECT_EQ(0, table.Find(PixelFormats::kEtc2R8G8B8UNorm).index);
  EXPECT_EQ(1, table.Find(PixelFormats::kEtc2R8G8B8Srgb).index);
  EXPECT_EQ(5, table.Find(PixelFormats::kEtc2R8G8B8A8Srgb).index);
}

// Tests using PixelFormatTable with a stride.
TEST(PixelFormatTest, PixelFormatTableStride) {
  struct MyEntry {
    int index;
    MyEntry(const MyEntry& other) = delete;  // Ensure we don't copy.
  };
  static const MyEntry kTable[] = {
      {0}, {2}, {4},
  };
  PixelFormatTable<MyEntry, PixelFormats::kEtc2R8G8B8UNorm,
                   PixelFormats::kEtc2R8G8B8A8Srgb, 2>
      table(kTable);
  EXPECT_EQ(6, table.range());
  EXPECT_EQ(3, table.size());
  EXPECT_EQ(0, table.Find(PixelFormats::kEtc2R8G8B8UNorm).index);
  EXPECT_EQ(0, table.Find(PixelFormats::kEtc2R8G8B8Srgb).index);
  EXPECT_EQ(4, table.Find(PixelFormats::kEtc2R8G8B8A8Srgb).index);
}

// Tests some uncompressed types and their math.
TEST(PixelFormatTest, UncompressedTypes) {
  PixelFormat format = PixelFormats::kA2B10G10R10UNorm;
  EXPECT_EQ(19, format.unique_id());
  EXPECT_EQ(PixelPacking::kUncompressed, format.packing_format());
  EXPECT_EQ(ComponentFormat::kUNorm, format.component_format());
  EXPECT_EQ(4, format.component_count());
  EXPECT_EQ(10, format.component_bits_r());
  EXPECT_EQ(10, format.component_bits_g());
  EXPECT_EQ(10, format.component_bits_b());
  EXPECT_EQ(2, format.component_bits_a());
  EXPECT_FALSE(format.is_flexible());
  EXPECT_FALSE(format.is_compressed());
  EXPECT_FALSE(format.is_depth_stencil());
  EXPECT_TRUE(format.has_transparency());
  EXPECT_TRUE(format.is_linear());
  EXPECT_EQ(4 * 100 * 100, format.ComputeDataSize(100, 100));
}

// Tests some depth stencil types and their math.
TEST(PixelFormatTest, DepthStencilTypes) {
  PixelFormat format = PixelFormats::kD24UNormS8UInt;
  EXPECT_EQ(53, format.unique_id());
  EXPECT_EQ(PixelPacking::kDepthStencil, format.packing_format());
  EXPECT_EQ(ComponentFormat::kUNorm, format.component_format());
  EXPECT_EQ(2, format.component_count());
  EXPECT_EQ(24, format.component_bits_depth());
  EXPECT_EQ(8, format.component_bits_stencil());
  EXPECT_FALSE(format.is_flexible());
  EXPECT_FALSE(format.is_compressed());
  EXPECT_TRUE(format.is_depth_stencil());
  EXPECT_FALSE(format.has_transparency());
  EXPECT_TRUE(format.is_linear());
  EXPECT_EQ(4 * 100 * 100, format.ComputeDataSize(100, 100));
}

// Tests some compressed types and their math.
TEST(PixelFormatTest, CompressedTypes) {
  PixelFormat format = PixelFormats::kAstc4x4Srgb;
  EXPECT_EQ(80, format.unique_id());
  EXPECT_EQ(PixelPacking::kAstc, format.packing_format());
  EXPECT_EQ(ComponentFormat::kSrgb, format.component_format());
  EXPECT_EQ(4, format.component_count());
  EXPECT_EQ(1, format.component_bits_r());
  EXPECT_EQ(1, format.component_bits_g());
  EXPECT_EQ(1, format.component_bits_b());
  EXPECT_EQ(1, format.component_bits_a());
  EXPECT_FALSE(format.is_flexible());
  EXPECT_TRUE(format.is_compressed());
  EXPECT_FALSE(format.is_depth_stencil());
  EXPECT_TRUE(format.has_transparency());
  EXPECT_FALSE(format.is_linear());
  EXPECT_EQ(100 * 100, format.ComputeDataSize(100, 100));

  // Some more for code coverage.
  EXPECT_EQ(5000, PixelFormats::kBC1RGBAUNorm.ComputeDataSize(100, 100));
  EXPECT_EQ(10000, PixelFormats::kBC2UNorm.ComputeDataSize(100, 100));
  EXPECT_EQ(5000, PixelFormats::kEtc2R8G8B8UNorm.ComputeDataSize(100, 100));
  EXPECT_EQ(10000, PixelFormats::kEtc2R8G8B8A8UNorm.ComputeDataSize(100, 100));
}

}  // namespace
}  // namespace gfx
}  // namespace xrtl
