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

#ifndef XRTL_GFX_RESOURCE_H_
#define XRTL_GFX_RESOURCE_H_

#include <cstdint>

#include "xrtl/gfx/managed_object.h"

namespace xrtl {
namespace gfx {

class MemoryHeap;

// A bitmask specifying properties for a memory type.
enum class MemoryType {
  // Memory allocated with this type is the most efficient for device access.
  kDeviceLocal = 1 << 0,

  // Memory allocated with this type can be mapped for host access using
  // Resource::MapMemory.
  kHostVisible = 1 << 1,

  // The host cache management commands MappedMemory::Flush and
  // MappedMemory::Invalidate are not needed to flush host writes
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

// Base type for allocated resources.
class Resource : public ManagedObject {
 public:
  // The memory heap this resource was allocated from.
  // The heap will be kept alive so long as this resource remains allocated.
  virtual ref_ptr<MemoryHeap> memory_heap() const = 0;

  // Size of the resource memory allocation in bytes.
  // This may be rounded up from the originally requested size or the ideal
  // size for the resource based on device restrictions.
  size_t allocation_size() const { return allocation_size_; }

 protected:
  explicit Resource(size_t allocation_size)
      : allocation_size_(allocation_size) {}

  size_t allocation_size_ = 0;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RESOURCE_H_
