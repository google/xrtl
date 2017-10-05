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

#ifndef XRTL_TESTING_DIFFING_IMAGE_DIFFER_H_
#define XRTL_TESTING_DIFFING_IMAGE_DIFFER_H_

#include <memory>
#include <utility>

#include "xrtl/testing/image_buffer.h"

namespace xrtl {
namespace testing {
namespace diffing {

// Utilities for diffing images.
class ImageDiffer {
 public:
  enum Mode {
    // TODO(benvanik): more modes (perceptual, rms error, etc).
    kDefault = 0,
  };

  // Options that can be used to adjust the tolerance of an image comparison
  // operation or how the images are compared.
  struct Options {
    // Comparison mode.
    Mode mode = Mode::kDefault;
    // Euclidean distance in RGB colorspace.
    int allowable_per_pixel_color_difference = 0;
    // Maximum number of pixels allowed to differ.
    int allowable_number_pixels_different = 0;
  };

  // Result of a diffing operation.
  struct Result {
    // True if the expected and actual image buffers were equivalent
    // as defined by the comparison mode and options.
    bool equivalent = false;

    // TODO(benvanik): diff image/heatmap/etc.
  };

  // Diffs an image against its expected value and returns the result.
  static Result DiffImageBuffers(ImageBuffer* expected_image_buffer,
                                 ImageBuffer* actual_image_buffer,
                                 Options options);
  static Result DiffImageBuffers(
      std::unique_ptr<ImageBuffer> expected_image_buffer,
      ImageBuffer* actual_image_buffer, Options options) {
    return DiffImageBuffers(expected_image_buffer.get(), actual_image_buffer,
                            std::move(options));
  }

  // Compares two images against each other.
  // Returns false if the test image does not match the expected image based on
  // the provided options.
  static bool CompareImageBuffers(ImageBuffer* expected_image_buffer,
                                  ImageBuffer* actual_image_buffer,
                                  Options options) {
    return DiffImageBuffers(expected_image_buffer, actual_image_buffer,
                            std::move(options))
        .equivalent;
  }
  static bool CompareImageBuffers(
      std::unique_ptr<ImageBuffer> expected_image_buffer,
      ImageBuffer* actual_image_buffer, Options options) {
    return CompareImageBuffers(expected_image_buffer.get(), actual_image_buffer,
                               std::move(options));
  }
};

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_DIFFING_IMAGE_DIFFER_H_
