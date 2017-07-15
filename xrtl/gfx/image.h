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

#ifndef XRTL_GFX_IMAGE_H_
#define XRTL_GFX_IMAGE_H_

#include "xrtl/base/geometry.h"
#include "xrtl/gfx/render_state.h"
#include "xrtl/gfx/resource.h"

namespace xrtl {
namespace gfx {

class ImageView;

// An image resource.
class Image : public Resource {
 public:
  // Defines the base type and dimensionality of an image.
  enum class Type {
    // A two-dimensional image.
    k2D = 0,
    // An array of two-dimensional images.
    k2DArray = 1,
    // A three-dimensional image.
    k3D = 2,
    // A cube image with six two-dimensional images.
    //
    // Layer mapping:
    //   0: +X
    //   1: -X
    //   2: +Y
    //   3: -Y
    //   4: +Z
    //   5: -Z
    kCube = 3,
  };

  // Defines how an image is intended to be used.
  enum class Usage {
    kNone = 0,
    // Indicates that the image can be used as the source of a transfer
    // command.
    kTransferSource = 0x00000001,
    // Indicates that the image can be used as the target of a transfer
    // command.
    kTransferTarget = 0x00000002,
    // Indicates that the image can be used in a ResourceSet as a
    // kSampledImage or kCombinedImageSampler.
    kSampled = 0x00000004,
    // Indicates that the image can be used in a ResourceSet as a
    // kStorageImage.
    kStorage = 0x00000008,
    // Indicates that the image can be used as an attachment in a Framebuffer.
    kColorAttachment = 0x00000010,
    // Indicates that the image can be used as an attachment in a Framebuffer.
    kDepthStencilAttachment = 0x00000020,
    // Indicates that the memory bound to this image will have been allocated
    // with the MemoryType::kLazilyAllocated bit.
    kTransientAttachment = 0x00000040,
    // Indicates that the image can be used in a ResourceSet as a
    // kInputAttachment, be read from a shader as an input attachment, and be
    // used as an input attachment in a Framebuffer.
    kInputAttachment = 0x00000080,
  };

  // Specifies the tiling arrangement of data in an image.
  enum class TilingMode {
    // Texels are laid out in an implementation-dependent arrangement for more
    // optimal memory access.
    kOptimal = 0,
    // Texels are laid out in memory in row-major order possibly with some
    // padding on each row.
    kLinear = 1,
  };

  // Layout of the pixel data memory on the device.
  // Images must be put into a compatible layout before certain operations
  // are allowed. kGeneral is usually supported, however one of the Optimal
  // layouts will usually give better performance. Use ImageBarrier to perform
  // layout transitions.
  enum class Layout {
    kUndefined = 0,
    kGeneral = 1,
    kColorAttachmentOptimal = 2,
    kDepthStencilAttachmentOptimal = 3,
    kDepthStencilReadOnlyOptimal = 4,
    kShaderReadOnlyOptimal = 5,
    kTransferSourceOptimal = 6,
    kTransferTargetOptimal = 7,
    kPreinitialized = 8,
    kPresentSource = 1000001002,
  };

  // Bitmask specifying which aspects of an image are included in a view.
  enum class AspectFlag : uint32_t {
    kColor = 0x00000001,
    kDepth = 0x00000002,
    kStencil = 0x00000004,
    kDepthStencil = kDepth | kStencil,
  };

  // Defines a range of layers within the image.
  struct LayerRange {
    // Selects whether color, depth, and/or stencil aspects will be used.
    AspectFlag aspect_mask = AspectFlag::kColor;
    // Mipmap level source/target.
    int mip_level = 0;
    // Starting layer index.
    int base_layer = 0;
    // Total layer count.
    int layer_count = 0;

    LayerRange() = default;
    LayerRange(AspectFlag aspect_mask, int mip_level, int base_layer,
               int layer_count)
        : aspect_mask(aspect_mask),
          mip_level(mip_level),
          base_layer(base_layer),
          layer_count(layer_count) {}
  };

  struct CreateParams {
    Type type = Image::Type::k2D;
    PixelFormat format = PixelFormats::kUndefined;
    SampleCount sample_count = SampleCount::k1;
    TilingMode tiling_mode = TilingMode::kOptimal;
    Size3D size;
    int mip_level_count = 1;
    int array_layer_count = 1;
    Layout initial_layout = Layout::kUndefined;
  };

  ~Image() override = default;

  // Bitmask describing how the image is to be used.
  Usage usage_mask() const { return usage_mask_; }

  // Image type the view is representing.
  Image::Type type() const { return create_params_.type; }
  // Format of the pixel data.
  PixelFormat format() const { return create_params_.format; }
  // The number of samples of the image, if it is to be multisampled.
  SampleCount sample_count() const { return create_params_.sample_count; }
  // Tiling mode of the image.
  TilingMode tiling_mode() const { return create_params_.tiling_mode; }
  // Size of the image in pixels of each valid dimension.
  Size3D size() const { return create_params_.size; }
  // Total number of levels of detail for mipmapping.
  // A count of 1 indicates that no mipmapping is to be performed.
  int mip_level_count() const { return create_params_.mip_level_count; }
  // Total number of layers in the array, if the image is an array type.
  int array_layer_count() const { return create_params_.array_layer_count; }

  // Returns a layer range encompassing the entire image.
  LayerRange entire_range() const {
    return {create_params_.format.is_depth_stencil() ? AspectFlag::kDepthStencil
                                                     : AspectFlag::kColor,
            0, 0, create_params_.array_layer_count};
  }

  // Creates a new image view referencing an existing image.
  virtual ref_ptr<ImageView> CreateView() = 0;
  virtual ref_ptr<ImageView> CreateView(Image::Type type, PixelFormat format,
                                        Image::LayerRange layer_range) = 0;
  virtual ref_ptr<ImageView> CreateView(Image::Type type,
                                        PixelFormat format) = 0;

  // Reads a block of data from the resource at the given source layer range.
  //
  // Returns false if the read could not be performed; either the bounds are
  // out of range or the memory type does not support reading in this way.
  virtual bool ReadData(LayerRange source_range, void* data,
                        size_t data_length) = 0;

  // Writes a block of data into the image at the given target layer range.
  //
  // Returns false if the write could not be performed; either the bounds are
  // out of range or the memory type does not support writing in this way.
  virtual bool WriteData(LayerRange target_range, const void* data,
                         size_t data_length) = 0;

 protected:
  Image(size_t allocation_size, CreateParams create_params)
      : Resource(allocation_size), create_params_(create_params) {}

  Usage usage_mask_ = Usage::kNone;
  CreateParams create_params_;
};

XRTL_BITMASK(Image::Usage);
XRTL_BITMASK(Image::AspectFlag);

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_IMAGE_H_
