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

#ifndef XRTL_GFX_MEMORY_POOL_H_
#define XRTL_GFX_MEMORY_POOL_H_

#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/buffer.h"
#include "xrtl/gfx/image.h"
#include "xrtl/gfx/resource.h"

namespace xrtl {
namespace gfx {

// A bitmask specifying properties for a memory type.
enum class MemoryType {
  // Memory allocated with this type is the most efficient for device access.
  kDeviceLocal = 1 << 0,

  // Memory allocated with this type can be mapped for host access using
  // Resource::MapMemory.
  kHostVisible = 1 << 1,

  // The host cache management commands Resource::FlushMappedMemory and
  // Resource::InvalidateMappedMemory are not needed to flush host writes
  // to the device or make device writes visible to the host, respectively.
  kHostCoherent = 1 << 2,

  // Memory allocated with this type is cached on the host. Host memory accesses
  // to uncached memory are slower than to cached memory, however uncached
  // memory is always host coherent.
  kHostCached = 1 << 3,

  // Memory is lazily allocated by the hardware and only exists transiently.
  // This is the optimal mode for memory used only between subpasses in the same
  // render pass, as it can often be kept entirely on-tile and discard when the
  // render pass ends.
  // The memory type only allows device access to the memory. Memory types must
  // not have both this and kHostVisible set.
  kLazilyAllocated = 1 << 4,
};
XRTL_BITMASK(MemoryType);

// Memory pool for images and buffers.
// Allocations that require reaching into the device to allocate memory are
// expensive and there may be limits on the number of allocations that can be
// performed by a process (sometimes on the order of low hundreds). MemoryPools
// work around this by allocating large chunks of memory and then handing out
// slices of that when requested as buffers or images.
//
// MemoryPools and the Resources allocated from them must be kept alive together
// and the ref_ptr system should take care of this. This means that callers must
// be careful not to allow Resources to hang around longer than required as it
// may keep large chunks of memory reserved by a no-longer-used allocator.
class MemoryPool : public RefObject<MemoryPool> {
 public:
  virtual ~MemoryPool() = default;

  // TODO(benvanik): expose stats (total chunk size, chunks allocated, etc).
  // TODO(benvanik): expose properties to query granularities for mapping/etc.

  // A bitmask of MemoryType values describing the behavior of this memory.
  MemoryType memory_type_mask() const { return memory_type_mask_; }
  // Size of each chunk the allocator uses for backing memory in bytes.
  size_t chunk_size() const { return chunk_size_; }

  // Attempts to reclaim unused chunks from the system.
  // Chunks will not be reclaimed so long as any resources allocated within it
  // are still alive.
  virtual void Reclaim() = 0;

  // Defines the result of an allocation request.
  enum class AllocationResult {
    // Allocation was successful and the out param contains the new resource.
    kSuccess,
    // Invalid creation arguments, such as a nonsensical format or invalid size.
    kInvalidArguments,
    // The requested allocation makes sense is not supported by the current
    // context.
    kUnsupported,
    // One or more device limits were exceeded by the specified parameters.
    kLimitsExceeded,
    // Device memory allocation would have been over the size of a single chunk.
    // Grow the chunk size or use a different allocator.
    kOverChunkSize,
    // The memory pool servicing the memory type is exhausted.
    kOutOfMemory,
  };

  // Allocates a buffer from the allocator memory pool.
  // Returns the buffer via out_buffer if it could be allocated successfully.
  // The allocation may fail if the buffer is larger than the allocator chunk
  // size, memory is not available, or the buffer parameters are invalid or
  // unsupported.
  virtual AllocationResult AllocateBuffer(size_t size, Buffer::Usage usage_mask,
                                          ref_ptr<Buffer>* out_buffer) = 0;

  // Allocates an image from the allocator memory pool.
  // Returns the image via out_image if it could be allocated successfully.
  // The allocation may fail if the image is larger than the allocator chunk
  // size, memory is not available, or the image parameters are invalid or
  // unsupported.
  virtual AllocationResult AllocateImage(Image::CreateParams create_params,
                                         ref_ptr<Image>* out_image) = 0;

 protected:
  MemoryPool(MemoryType memory_type_mask, size_t chunk_size)
      : memory_type_mask_(memory_type_mask), chunk_size_(chunk_size) {}

  MemoryType memory_type_mask_;
  size_t chunk_size_ = 0;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_MEMORY_POOL_H_
