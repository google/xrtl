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

#include "xrtl/gfx/es3/es3_image.h"

#include <utility>

#include "xrtl/gfx/es3/es3_image_view.h"
#include "xrtl/gfx/memory_heap.h"

namespace xrtl {
namespace gfx {
namespace es3 {

size_t ES3Image::ComputeAllocationSize(
    const Image::CreateParams& create_params) {
  size_t allocation_size = create_params.format.ComputeDataSize(
      create_params.size.width, create_params.size.height);
  switch (create_params.type) {
    case Image::Type::k2D:
      allocation_size *= create_params.mip_level_count;
      break;
    case Image::Type::kCube:
      allocation_size *= create_params.mip_level_count;
      allocation_size *= 6;
      break;
    case Image::Type::k2DArray:
      allocation_size *= create_params.mip_level_count;
      allocation_size *= create_params.array_layer_count;
      break;
    case Image::Type::k3D:
      allocation_size *= create_params.size.depth;
      allocation_size *= create_params.mip_level_count;
      break;
  }
  return allocation_size;
}

ES3Image::ES3Image(ref_ptr<ES3PlatformContext> platform_context,
                   ref_ptr<MemoryHeap> memory_heap,
                   ES3TextureParams texture_params, size_t allocation_size,
                   CreateParams create_params)
    : Image(allocation_size, create_params),
      platform_context_(std::move(platform_context)),
      memory_heap_(std::move(memory_heap)),
      texture_params_(texture_params) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

  // TODO(benvanik): pool ID allocation.
  glGenTextures(1, &texture_id_);

  static const GLenum kTypeTarget[] = {
      GL_TEXTURE_2D,        // Image::Type::k2D
      GL_TEXTURE_2D_ARRAY,  // Image::Type::k2DArray
      GL_TEXTURE_3D,        // Image::Type::k3D
      GL_TEXTURE_CUBE_MAP,  // Image::Type::kCube
  };
  DCHECK_LT(static_cast<int>(create_params.type), count_of(kTypeTarget));
  target_ = kTypeTarget[static_cast<int>(create_params.type)];
  glBindTexture(target_, texture_id_);

  // Allocate storage for the texture data.
  switch (create_params.type) {
    case Image::Type::k2D:
    case Image::Type::kCube:
      glTexStorage2D(target_, create_params.mip_level_count,
                     texture_params_.internal_format, create_params.size.width,
                     create_params.size.height);
      break;
    case Image::Type::k2DArray:
      glTexStorage3D(target_, create_params.mip_level_count,
                     texture_params_.internal_format, create_params.size.width,
                     create_params.size.height,
                     create_params.array_layer_count);
      break;
    case Image::Type::k3D:
      glTexStorage3D(target_, create_params.mip_level_count,
                     texture_params_.internal_format, create_params.size.width,
                     create_params.size.height, create_params.size.depth);
      break;
  }

  // Set default sampling parameters.
  // We'll use Sampler objects to perform the sampling, but to use the texture
  // as render target we need to ensure it doesn't have mip mapping set.
  glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindTexture(target_, 0);
}

ES3Image::~ES3Image() {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
  glDeleteTextures(1, &texture_id_);
}

ref_ptr<MemoryHeap> ES3Image::memory_heap() const { return memory_heap_; }

void ES3Image::Release() { memory_heap_->ReleaseImage(this); }

ref_ptr<ImageView> ES3Image::CreateView() {
  return CreateView(create_params_.type, create_params_.format, entire_range());
}

ref_ptr<ImageView> ES3Image::CreateView(Image::Type type, PixelFormat format,
                                        Image::LayerRange layer_range) {
  return make_ref<ES3ImageView>(ref_ptr<Image>(this), type, format,
                                layer_range);
}

ref_ptr<ImageView> ES3Image::CreateView(Image::Type type, PixelFormat format) {
  return CreateView(type, format, entire_range());
}

bool ES3Image::ReadData(LayerRange source_range, void* data,
                        size_t data_length) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

  // TODO(benvanik): support automatically splitting across layers.
  DCHECK_EQ(1, source_range.layer_count);

  // TODO(benvanik): image.
  DCHECK(false);
  return false;
}

bool ES3Image::WriteData(LayerRange target_range, const void* data,
                         size_t data_length) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

  // TODO(benvanik): support automatically splitting across layers.
  //                 We'll need to shift around in data for each layer.
  DCHECK_EQ(1, target_range.layer_count);

  GLenum target = target_;
  int level = target_range.mip_level;
  if (type() == Type::kCube) {
    // Special cubemap handling, where layer index changes the target.
    DCHECK_LT(target_range.base_layer, 6);
    target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + target_range.base_layer;
  }

  // TODO(benvanik): support arrays/3D textures.
  DCHECK(type() == Type::k2D || type() == Type::kCube);
  // TODO(benvanik): support compressed texture types.
  DCHECK_NE(texture_params_.type, GL_NONE);

  // Upload image.
  glBindTexture(target, texture_id_);
  glTexSubImage2D(target, level, 0, 0, size().width, size().height,
                  texture_params_.format, texture_params_.type, data);
  glBindTexture(target, 0);

  return true;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
