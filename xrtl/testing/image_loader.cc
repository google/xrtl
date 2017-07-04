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

#include "xrtl/testing/image_loader.h"

#include <stb_image.h>

#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/testing/file_manifest.h"

namespace xrtl {
namespace testing {

Image ImageLoader::LoadImage(StringView path, int image_channels) {
  Image image;
  ImageDataPtr image_data = {
      stbi_load(FileManifest::ResolvePath(path).data(), &image.width,
                &image.height, &image.channels, image_channels),
      [](uint8_t* data) { stbi_image_free(data); }};
  image.data = std::move(image_data);

  if (!image.data) {
    LOG(ERROR) << "Couldn't load the image at '" << path.data() << "'";
  }

  return image;
}

}  // namespace testing
}  // namespace xrtl
