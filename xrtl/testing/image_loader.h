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

#ifndef XRTL_TESTING_IMAGE_LOADER_H_
#define XRTL_TESTING_IMAGE_LOADER_H_

#include <memory>

#include "xrtl/base/string_view.h"

namespace xrtl {
namespace testing {

using ImageDataPtr = std::unique_ptr<uint8_t, void (*)(uint8_t*)>;

struct Image {
  ImageDataPtr data = {nullptr, nullptr};
  int width = 0;
  int height = 0;
  int channels = 0;
};

// Utilities for loading images from the file system.
class ImageLoader {
 public:
  // Loads the image at the specified path with the desired number of channels.
  // If the image failed to load, its fields will be empty.
  static Image LoadImage(StringView path, int image_channels);
};

}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_IMAGE_LOADER_H_
