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

#ifndef XRTL_TESTING_IMAGE_COMPARISON_H_
#define XRTL_TESTING_IMAGE_COMPARISON_H_

#include <string>

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace testing {

// Utilities for comparing images.
class ImageComparison {
 public:
  // Returns false if the test image does not exactly match the expected image.
  static bool CompareImages(const void* expected_image_data,
                            int expected_image_width, int expected_image_height,
                            const void* test_image_data, int test_image_width,
                            int test_image_height, int channels);
};

}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_IMAGE_COMPARISON_H_
