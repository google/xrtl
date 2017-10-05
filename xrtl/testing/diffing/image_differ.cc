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

#include "xrtl/base/logging.h"

namespace xrtl {
namespace testing {
namespace diffing {

ImageDiffer::Result ImageDiffer::DiffImageBuffers(
    ImageBuffer* expected_image_buffer, ImageBuffer* actual_image_buffer,
    Options options) {
  if (!expected_image_buffer || !actual_image_buffer) {
    LOG(ERROR) << "One or more input buffers nullptr";
    return Result{};
  }

  Result result;
  result.equivalent = true;

  if (expected_image_buffer->data_width() !=
          actual_image_buffer->data_width() ||
      expected_image_buffer->data_height() !=
          actual_image_buffer->data_height()) {
    LOG(ERROR) << "Expected data dimensions of "
               << expected_image_buffer->data_width() << "x"
               << expected_image_buffer->data_height() << " but got actual "
               << actual_image_buffer->data_width() << "x"
               << actual_image_buffer->data_height();
    result.equivalent = false;
  }
  if (expected_image_buffer->display_width() !=
          actual_image_buffer->display_width() ||
      expected_image_buffer->display_height() !=
          actual_image_buffer->display_height()) {
    LOG(ERROR) << "Expected display dimensions of "
               << expected_image_buffer->display_width() << "x"
               << expected_image_buffer->display_height() << " but got actual "
               << actual_image_buffer->display_width() << "x"
               << actual_image_buffer->display_height();
    result.equivalent = false;
  }
  if (expected_image_buffer->channels() != actual_image_buffer->channels()) {
    LOG(ERROR) << "Expected " << expected_image_buffer->channels()
               << " channels but got actual "
               << actual_image_buffer->channels();
    result.equivalent = false;
  }
  if (expected_image_buffer->row_stride() !=
      actual_image_buffer->row_stride()) {
    LOG(ERROR) << "Expected row stride of "
               << expected_image_buffer->row_stride() << " but got actual "
               << actual_image_buffer->row_stride();
    result.equivalent = false;
  }
  if (expected_image_buffer->data_size() != actual_image_buffer->data_size()) {
    LOG(ERROR) << "Expected data size of " << expected_image_buffer->data_size()
               << " but got actual " << actual_image_buffer->data_size();
    result.equivalent = false;
  }
  if (expected_image_buffer->data_size() == actual_image_buffer->data_size()) {
    // Note that we avoid memcmp if sizes differ.
    bool is_data_equal =
        std::memcmp(expected_image_buffer->data(), actual_image_buffer->data(),
                    expected_image_buffer->data_size()) == 0;
    if (!is_data_equal) {
      LOG(ERROR) << "One or more bytes differ";
      result.equivalent = false;
    }
  }

  return result;
}

}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
