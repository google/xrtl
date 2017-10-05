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

#include "xrtl/testing/diffing/image_differ.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace testing {
namespace diffing {
namespace {

// Tests that an image equals itself.
TEST(ImageDifferTest, SameImage) {
  int channels = 3;

  auto expected_image_buffer = ImageBuffer::Load(
      "xrtl/testing/diffing/testdata/image_file.png", channels);
  ASSERT_NE(nullptr, expected_image_buffer);

  auto test_image_buffer = ImageBuffer::Load(
      "xrtl/testing/diffing/testdata/image_file.png", channels);
  ASSERT_NE(nullptr, test_image_buffer);

  EXPECT_TRUE(ImageDiffer::CompareImageBuffers(expected_image_buffer.get(),
                                               test_image_buffer.get(), {}));
}

// Tests that different images with the same size are considered different.
TEST(ImageDifferTest, DifferentImage) {
  int channels = 3;

  auto expected_image_buffer = ImageBuffer::Load(
      "xrtl/testing/diffing/testdata/image_file.png", channels);
  ASSERT_NE(nullptr, expected_image_buffer);

  auto test_image_buffer = ImageBuffer::Load(
      "xrtl/testing/diffing/testdata/image_file_mismatch.png", channels);
  ASSERT_NE(nullptr, test_image_buffer);

  EXPECT_FALSE(ImageDiffer::CompareImageBuffers(expected_image_buffer.get(),
                                                test_image_buffer.get(), {}));
}

}  // namespace
}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
