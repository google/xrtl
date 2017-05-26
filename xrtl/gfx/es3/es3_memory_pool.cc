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

#include "xrtl/gfx/es3/es3_memory_pool.h"

#include <utility>

#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_buffer.h"
#include "xrtl/gfx/es3/es3_image.h"
#include "xrtl/gfx/es3/es3_pixel_format.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3MemoryPool::ES3MemoryPool(ref_ptr<ES3PlatformContext> platform_context,
                             MemoryType memory_type_mask, size_t chunk_size)
    : MemoryPool(memory_type_mask, chunk_size),
      platform_context_(platform_context) {}

ES3MemoryPool::~ES3MemoryPool() = default;

void ES3MemoryPool::Reclaim() {
  WTF_SCOPE0("ES3MemoryPool#Reclaim");

  // TODO(benvanik): reclaim unused resources.
}

MemoryPool::AllocationResult ES3MemoryPool::AllocateBuffer(
    size_t size, Buffer::Usage usage_mask, ref_ptr<Buffer>* out_buffer) {
  WTF_SCOPE0("ES3MemoryPool#AllocateBuffer");

  // Create the buffer and allocate underlying storage.
  *out_buffer = make_ref<ES3Buffer>(platform_context_, size, usage_mask);

  return AllocationResult::kSuccess;
}

MemoryPool::AllocationResult ES3MemoryPool::AllocateImage(
    Image::CreateParams create_params, ref_ptr<Image>* out_image) {
  WTF_SCOPE0("ES3MemoryPool#AllocateImage");

  // Pick a pixel format for the texture.
  GLenum internal_format =
      ConvertPixelFormatToInternalFormat(create_params.format);
  if (internal_format == GL_NONE) {
    LOG(ERROR) << "Unsupported GL pixel format";
    return AllocationResult::kUnsupported;
  }

  // TODO(benvanik): validate limits.

  // Compute allocated data size (once uploaded). This is how much memory the
  // image will consume on the GPU (at least).
  size_t allocation_size = ES3Image::ComputeAllocationSize(create_params);

  // Create the image and allocate underlying texture.
  *out_image = make_ref<ES3Image>(platform_context_, internal_format,
                                  allocation_size, std::move(create_params));

  return AllocationResult::kSuccess;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
