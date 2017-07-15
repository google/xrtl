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

#include "xrtl/gfx/es3/es3_memory_heap.h"

#include <utility>

#include "xrtl/base/math.h"
#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_buffer.h"
#include "xrtl/gfx/es3/es3_image.h"
#include "xrtl/gfx/es3/es3_pixel_format.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3MemoryHeap::ES3MemoryHeap(ref_ptr<ES3PlatformContext> platform_context,
                             MemoryType memory_type_mask, size_t heap_size)
    : MemoryHeap(memory_type_mask, heap_size),
      platform_context_(std::move(platform_context)) {
  // TODO(benvanik): query? or always leave like this? (common in vk).
  allocation_alignment_ = 128;
}

ES3MemoryHeap::~ES3MemoryHeap() {
  std::lock_guard<std::mutex> lock_guard(mutex_);
  DCHECK_EQ(used_size_, 0);
}

size_t ES3MemoryHeap::used_size() {
  std::lock_guard<std::mutex> lock_guard(mutex_);
  return used_size_;
}

MemoryHeap::AllocationResult ES3MemoryHeap::AllocateBuffer(
    size_t size, Buffer::Usage usage_mask, ref_ptr<Buffer>* out_buffer) {
  WTF_SCOPE0("ES3MemoryHeap#AllocateBuffer");

  // Ensure we can allocate the requested amount.
  size_t allocation_size = math::RoundToAlignment(size, allocation_alignment_);
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (used_size_ + allocation_size > heap_size_) {
      return AllocationResult::kOutOfMemory;
    }
    used_size_ += allocation_size;
  }

  // Create the buffer and allocate underlying storage.
  *out_buffer =
      make_ref<ES3Buffer>(platform_context_, ref_ptr<MemoryHeap>(this),
                          allocation_size, usage_mask);

  return AllocationResult::kSuccess;
}

void ES3MemoryHeap::ReleaseBuffer(Buffer* buffer) {
  std::lock_guard<std::mutex> lock_guard(mutex_);
  used_size_ -= buffer->allocation_size();
}

MemoryHeap::AllocationResult ES3MemoryHeap::AllocateImage(
    Image::CreateParams create_params, Image::Usage usage_mask,
    ref_ptr<Image>* out_image) {
  WTF_SCOPE0("ES3MemoryHeap#AllocateImage");

  // Pick a pixel format for the texture.
  ES3TextureParams texture_params;
  if (!ConvertPixelFormatToTextureParams(create_params.format,
                                         &texture_params)) {
    LOG(ERROR) << "Unsupported GL pixel format";
    return AllocationResult::kUnsupported;
  }

  // TODO(benvanik): validate limits.

  // Compute allocated data size (once uploaded). This is how much memory the
  // image will consume on the GPU (at least).
  size_t allocation_size = ES3Image::ComputeAllocationSize(create_params);

  // Ensure we can allocate the requested amount.
  allocation_size =
      math::RoundToAlignment(allocation_size, allocation_alignment_);
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (used_size_ + allocation_size > heap_size_) {
      return AllocationResult::kOutOfMemory;
    }
    used_size_ += allocation_size;
  }

  // Create the image and allocate underlying texture.
  *out_image =
      make_ref<ES3Image>(platform_context_, ref_ptr<MemoryHeap>(this),
                         texture_params, allocation_size, create_params);

  return AllocationResult::kSuccess;
}

void ES3MemoryHeap::ReleaseImage(Image* image) {
  std::lock_guard<std::mutex> lock_guard(mutex_);
  used_size_ -= image->allocation_size();
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
