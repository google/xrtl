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

#ifndef XRTL_GFX_IMAGE_VIEW_H_
#define XRTL_GFX_IMAGE_VIEW_H_

#include <utility>

#include "xrtl/gfx/image.h"
#include "xrtl/gfx/managed_object.h"
#include "xrtl/gfx/pixel_format.h"

namespace xrtl {
namespace gfx {

// A view into an existing Image resource, possibly with a different type or
// format and some subregion of the layers available.
class ImageView : public ManagedObject {
 public:
  // Image this view is into.
  ref_ptr<Image> image() const { return image_; }
  // Image type the view is representing.
  // This is compatible with the underlying Image.
  Image::Type type() const { return type_; }
  // Format of the pixel data.
  // This is compatible with the underlying Image.
  PixelFormat format() const { return format_; }
  // Size of the image in pixels of each valid dimension.
  Size3D size() const { return image_->size(); }
  // Layer range within the target image.
  Image::LayerRange layer_range() const { return layer_range_; }

 protected:
  ImageView(ref_ptr<Image> image, Image::Type type, PixelFormat format,
            Image::LayerRange layer_range)
      : image_(std::move(image)),
        type_(type),
        format_(format),
        layer_range_(layer_range) {}

  ref_ptr<Image> image_;
  Image::Type type_ = Image::Type::k2D;
  PixelFormat format_ = PixelFormats::kUndefined;
  Image::LayerRange layer_range_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_IMAGE_VIEW_H_
