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

#include "xrtl/base/logging.h"

namespace xrtl {
namespace testing {

bool ImageComparison::CompareImages(const void* expected_image_data,
                                    int expected_image_width,
                                    int expected_image_height,
                                    const void* test_image_data,
                                    int test_image_width, int test_image_height,
                                    int channels) {
  if (test_image_width != expected_image_width) {
    LOG(ERROR) << "Image widths don't match. Expected " << expected_image_width
               << "px but test was " << test_image_width << "px";
    return false;
  }

  if (test_image_height != expected_image_height) {
    LOG(ERROR) << "Image heights don't match. Expected "
               << expected_image_height << "px but test was "
               << test_image_height << "px";
    return false;
  }

  // 1 byte per channel per pixel.
  size_t bytes_in_images = test_image_width * test_image_height * channels;

  bool result =
      std::memcmp(expected_image_data, test_image_data, bytes_in_images) == 0;

  if (!result) {
    LOG(ERROR) << "Test image did not exactly match expected image";
  }

  return result;
}

}  // namespace testing
}  // namespace xrtl
