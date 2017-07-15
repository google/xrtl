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

#ifndef XRTL_GFX_ES3_ES3_IMAGE_H_
#define XRTL_GFX_ES3_ES3_IMAGE_H_

#include <functional>

#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_pixel_format.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/image.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3Image : public Image {
 public:
  static size_t ComputeAllocationSize(const Image::CreateParams& create_params);

  ES3Image(ref_ptr<ES3PlatformContext> platform_context,
           ref_ptr<MemoryHeap> memory_heap, ES3TextureParams texture_params,
           size_t allocation_size, CreateParams create_params);
  ~ES3Image() override;

  ref_ptr<MemoryHeap> memory_heap() const override;

  GLenum target() const { return target_; }
  GLuint texture_id() const { return texture_id_; }

  ref_ptr<ImageView> CreateView() override;
  ref_ptr<ImageView> CreateView(Image::Type type, PixelFormat format,
                                Image::LayerRange layer_range) override;
  ref_ptr<ImageView> CreateView(Image::Type type, PixelFormat format) override;

  bool ReadData(LayerRange source_range, void* data,
                size_t data_length) override;

  bool WriteData(LayerRange target_range, const void* data,
                 size_t data_length) override;

  void Release() override;

 private:
  ref_ptr<ES3PlatformContext> platform_context_;
  ref_ptr<MemoryHeap> memory_heap_;
  ES3TextureParams texture_params_;

  GLenum target_ = GL_TEXTURE_2D;
  GLuint texture_id_ = 0;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_IMAGE_H_
