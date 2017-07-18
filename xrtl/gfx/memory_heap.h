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

#ifndef XRTL_GFX_MEMORY_HEAP_H_
#define XRTL_GFX_MEMORY_HEAP_H_

#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/buffer.h"
#include "xrtl/gfx/image.h"
#include "xrtl/gfx/resource.h"

namespace xrtl {
namespace gfx {

// Memory heap for images and buffers.
// Allocations that require reaching into the device to allocate memory are
// expensive and there may be limits on the number of allocations that can be
// performed by a process (sometimes on the order of low hundreds). MemoryHeaps
// work around this by allocating large chunks of memory and then handing out
// slices of that when requested as buffers or images.
//
// Note that its possible for internal fragmentation to decrease the total bytes
// that can be allocated from the heap. Ensure that resources allocated from the
// heap are of consistent sizes or have similar lifetimes to ensure
// fragmentation is kept to a minimum.
//
// Resources allocated from the heap may reserve more memory than requested
// if the heap has allocation alignment restrictions. Because of this
// Resource::allocation_size may differ from the requested size and total heap
// usage may exceed expectations. The native heap alignment can be queried with
// the allocation_alignment member, noting that it may differ among heap types.
//
// MemoryHeaps and the Resources allocated from them must be kept alive together
// and the ref_ptr system should take care of this. This means that callers must
// be careful not to allow Resources to hang around longer than required as it
// may keep large chunks of memory reserved by a no-longer-used allocator.
//
// MemoryHeaps are thread-safe and allocations may occur from multiple threads
// simultaneously. Note that because of races and fragmentation used_size()
// must not be used to make assumptions about whether an allocation will succeed
// and callers must always check the AllocationResult.
//
// MemoryHeap roughly maps to the follow backend concepts:
// - D3D12: ID3D12Heap
// - Metal: MTLHeap
// - Vulkan: VkMemoryHeap
class MemoryHeap : public RefObject<MemoryHeap> {
 public:
  virtual ~MemoryHeap() = default;

  // TODO(benvanik): expose stats (total chunk size, chunks allocated, etc).
  // TODO(benvanik): expose properties to query granularities for mapping/etc.

  // A bitmask of MemoryType values describing the behavior of this memory.
  MemoryType memory_type_mask() const { return memory_type_mask_; }
  // Byte alignment of resources allocated from the heap. Allocations will
  // start on and extend to addresses aligned with this value.
  virtual size_t allocation_alignment() const = 0;
  // Total size of the heap in bytes.
  size_t heap_size() const { return heap_size_; }
  // Total bytes currently allocated from the heap in bytes.
  virtual size_t used_size() = 0;

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
    // The memory pool servicing the memory type is exhausted.
    kOutOfMemory,
  };

  // Allocates a buffer from the heap.
  // Returns the buffer via out_buffer if it could be allocated successfully.
  // The allocation may fail if the buffer is larger than the maximum available
  // contigugous free heap memory block, or the buffer parameters are invalid or
  // unsupported.
  virtual AllocationResult AllocateBuffer(size_t size, Buffer::Usage usage_mask,
                                          ref_ptr<Buffer>* out_buffer) = 0;

  // Allocates an image from the allocator memory pool.
  // Returns the image via out_image if it could be allocated successfully.
  // The allocation may fail if the image is larger than the maximum available
  // contigugous free heap memory block, or the image parameters are invalid or
  // unsupported.
  virtual AllocationResult AllocateImage(Image::CreateParams create_params,
                                         Image::Usage usage_mask,
                                         ref_ptr<Image>* out_image) = 0;

  // TODO(benvanik): find a good way to hide these to all subclasses.
  virtual void ReleaseBuffer(Buffer* buffer) = 0;
  virtual void ReleaseImage(Image* image) = 0;

 protected:
  MemoryHeap(MemoryType memory_type_mask, size_t heap_size)
      : memory_type_mask_(memory_type_mask), heap_size_(heap_size) {}

  MemoryType memory_type_mask_;
  size_t heap_size_ = 0;
};

std::ostream& operator<<(std::ostream& stream,
                         const MemoryHeap::AllocationResult& value);

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_MEMORY_HEAP_H_
