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

#include "xrtl/base/ref_ptr.h"

namespace xrtl {
namespace gfx {

class MemoryHeap;

// Base type for allocated resources.
class Resource : public RefObject<Resource> {
 public:
  virtual ~Resource() = default;

  // The memory heap this resource was allocated from.
  // The heap will be kept alive so long as this resource remains allocated.
  virtual ref_ptr<MemoryHeap> memory_heap() const = 0;

  // Size of the resource memory allocation in bytes.
  // This may be rounded up from the originally requested size or the ideal
  // size for the resource based on device restrictions.
  size_t allocation_size() const { return allocation_size_; }

  static void Delete(Resource* resource) { resource->Release(); }

 protected:
  explicit Resource(size_t allocation_size)
      : allocation_size_(allocation_size) {}

  // Releases the resource back to its heap.
  virtual void Release() = 0;

  size_t allocation_size_ = 0;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RESOURCE_H_
