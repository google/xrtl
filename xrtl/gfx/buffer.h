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

#ifndef XRTL_GFX_BUFFER_H_
#define XRTL_GFX_BUFFER_H_

#include "xrtl/gfx/resource.h"

namespace xrtl {
namespace gfx {

// A buffer resource.
class Buffer : public Resource {
 public:
  // Defines how a buffer is intended to be used.
  enum class Usage {
    kNone = 0,
    // Indicates that the buffer can be used as the source of a transfer
    // command.
    kTransferSource = 0x00000001,
    // Indicates that the buffer can be used as the target of a transfer
    // command.
    kTransferTarget = 0x00000002,
    // Indicates that the buffer can be used in a DescriptorSet as a
    // kUniformTexelBuffer.
    //
    // Uniform texel buffers differ from uniform buffers in that they are
    // read-only and cached during shader execution as if they were texel
    // fetches. This means that they only benefit when all threadgroups are
    // accessing the same areas of the buffer concurrently. If the buffer is
    // accessed randomly (or by vertex/instance ID) use a kUniformBuffer
    // instead.
    kUniformTexelBuffer = 0x00000004,
    // Indicates that the buffer can be used in a DescriptorSet as a
    // kStorageTexelBuffer.
    kStorageTexelBuffer = 0x00000008,
    // Indicates that the buffer can be used in a DescriptorSet as a
    // kUniformBuffer.
    kUniformBuffer = 0x00000010,
    // Indicates that the buffer can be used in a DescriptorSet as a
    // kStorageBuffer.
    kStorageBuffer = 0x00000020,
    // Indicates that the buffer can be passed to BindIndexBuffer.
    kIndexBuffer = 0x00000040,
    // Indicates that the buffer can be passed to BindVertexBuffer(s).
    kVertexBuffer = 0x00000080,
    // Indicates that the buffer can be passed to one of the DrawIndirect
    // methods.
    kIndirectBuffer = 0x00000100,
  };

  ~Buffer() override = default;

  // Bitmask describing how the buffer is to be used.
  Usage usage_mask() const { return usage_mask_; }

 protected:
  Buffer(size_t allocation_size, Usage usage_mask)
      : Resource(allocation_size), usage_mask_(usage_mask) {}

  Usage usage_mask_ = Usage::kNone;
};

XRTL_BITMASK(Buffer::Usage);

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_BUFFER_H_
