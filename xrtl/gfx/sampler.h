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

#ifndef XRTL_GFX_SAMPLER_H_
#define XRTL_GFX_SAMPLER_H_

#include "xrtl/gfx/managed_object.h"

namespace xrtl {
namespace gfx {

class Sampler : public ManagedObject {
 public:
  // Specifies filters used for image lookups.
  enum class Filter {
    kNearest = 0,
    kLinear = 1,
  };

  // Specifies filters used for mipmap lookups.
  enum class MipmapMode {
    kNearest = 0,
    kLinear = 1,
  };

  // Specifies behavior of sampling with image coordinates outside an image.
  enum class AddressMode {
    kRepeat = 0,
    kMirroredRepeat = 1,
    kClampToEdge = 2,
    kClampToBorder = 3,
  };

  // Predefined border color modes.
  enum class BorderColor {
    kTransparentBlackFloat = 0,
    kTransparentBlackInt = 1,
    kOpaqueBlackFloat = 2,
    kOpaqueBlackInt = 3,
    kOpaqueWhiteFloat = 4,
    kOpaqueWhiteInt = 5,
  };

  // All sampler parameters.
  struct Params {
    // The magnification filter to apply to lookups.
    Filter mag_filter = Filter::kNearest;
    // The minification filter to apply to lookups.
    Filter min_filter = Filter::kNearest;
    // The mipmap filter to apply to lookups.
    MipmapMode mipmap_mode = MipmapMode::kNearest;

    // The addressing mode for lookups outside [0..1] range for each coordinate.
    AddressMode address_mode_u = AddressMode::kRepeat;
    AddressMode address_mode_v = AddressMode::kRepeat;
    AddressMode address_mode_w = AddressMode::kRepeat;

    // The bias to be added to mipmap LOD calculation and bias provided by
    // image sampling functions.
    float mip_lod_bias = 0.0f;
    // Values used to clamp the computed level-of-detail value.
    float min_lod = 0.0f;
    float max_lod = 0.0f;

    // True to enable anisotropic filtering.
    bool anisotropy_enable = false;
    // Anisotropy value clamp.
    float max_anisotropy = 1.0f;

    // Predefined border color used when kClampToBorder is enabled.
    BorderColor border_color = BorderColor::kTransparentBlackFloat;

    // TODO(benvanik): verify this can be supported everywhere.
    // bool unnormalized_coordinates = false;
  };

  // Sampler parameters.
  const Params& params() const { return params_; }

 protected:
  explicit Sampler(Params params) : params_(params) {}

  Params params_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_SAMPLER_H_
