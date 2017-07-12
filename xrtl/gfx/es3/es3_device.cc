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

#include "xrtl/gfx/es3/es3_device.h"

#include "xrtl/gfx/es3/es3_common.h"

namespace xrtl {
namespace gfx {
namespace es3 {

bool ES3Device::AdoptCurrentContext() {
  // TODO(benvanik): pull renderer/etc.
  type_ = DeviceType::kGpu;

  // Query limits.
  // TODO(benvanik): query limits.

  // Query feature support flags.
  // TODO(benvanik): other features.
  QuerySupportedPixelFormats(&features_.pixel_formats);

  return true;
}

void ES3Device::QuerySupportedPixelFormats(
    Features::PixelFormats* pixel_formats) {
  pixel_formats->packed_depth_stencil = GLAD_GL_OES_packed_depth_stencil == 1;
  pixel_formats->bc1_2_3 = GLAD_GL_EXT_texture_compression_s3tc == 1;
  pixel_formats->bc4_5_6_7 = false;
  pixel_formats->etc2 = true;
  pixel_formats->eac = true;
  pixel_formats->astc = GLAD_GL_KHR_texture_compression_astc_hdr == 1;
  pixel_formats->pvrtc = GLAD_GL_IMG_texture_compression_pvrtc == 1;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
