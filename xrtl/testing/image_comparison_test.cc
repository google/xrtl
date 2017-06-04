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

#include "xrtl/testing/image_comparison.h"

#include "xrtl/testing/gtest.h"
#include "xrtl/testing/image_loader.h"

namespace xrtl {
namespace testing {
namespace {

// Tests that an image equals itself.
TEST(ImageComparisonTest, SameImage) {
  int channels = 3;

  Image expected_image =
      ImageLoader::LoadImage("xrtl/testing/testdata/images/grid.png", channels);
  ASSERT_NE(nullptr, expected_image.data);
  ASSERT_EQ(channels, expected_image.channels);

  Image test_image =
      ImageLoader::LoadImage("xrtl/testing/testdata/images/grid.png", channels);
  ASSERT_NE(nullptr, test_image.data);
  ASSERT_EQ(channels, test_image.channels);

  EXPECT_TRUE(ImageComparison::CompareImages(
      expected_image.data.get(), expected_image.width, expected_image.height,
      test_image.data.get(), test_image.width, test_image.height, channels));
}

// Tests that a cropped image is considered different.
TEST(ImageComparisonTest, CroppedImage) {
  int channels = 3;

  Image expected_image =
      ImageLoader::LoadImage("xrtl/testing/testdata/images/grid.png", channels);
  ASSERT_NE(nullptr, expected_image.data);
  ASSERT_EQ(channels, expected_image.channels);

  Image test_image = ImageLoader::LoadImage(
      "xrtl/testing/testdata/images/grid_cropped.png", channels);
  ASSERT_NE(nullptr, test_image.data);
  ASSERT_EQ(channels, test_image.channels);

  EXPECT_FALSE(ImageComparison::CompareImages(
      expected_image.data.get(), expected_image.width, expected_image.height,
      test_image.data.get(), test_image.width, test_image.height, channels));
}

// Tests that different images with the same size are considered different.
TEST(ImageComparisonTest, DifferentImage) {
  int channels = 3;

  Image expected_image =
      ImageLoader::LoadImage("xrtl/testing/testdata/images/grid.png", channels);
  ASSERT_NE(nullptr, expected_image.data);
  ASSERT_EQ(channels, expected_image.channels);

  Image test_image = ImageLoader::LoadImage(
      "xrtl/testing/testdata/images/triangle.png", channels);
  ASSERT_NE(nullptr, test_image.data);
  ASSERT_EQ(channels, test_image.channels);

  EXPECT_FALSE(ImageComparison::CompareImages(
      expected_image.data.get(), expected_image.width, expected_image.height,
      test_image.data.get(), test_image.width, test_image.height, channels));
}

}  // namespace
}  // namespace testing
}  // namespace xrtl
