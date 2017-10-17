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

#ifndef XRTL_GFX_ES3_ES3_BUFFER_H_
#define XRTL_GFX_ES3_ES3_BUFFER_H_

#include <functional>

#include "xrtl/gfx/buffer.h"
#include "xrtl/gfx/context.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_queue_object.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3Buffer : public Buffer, public ES3QueueObject {
 public:
  ES3Buffer(ES3ObjectLifetimeQueue* queue, ref_ptr<MemoryHeap> memory_heap,
            size_t allocation_size, Usage usage_mask);
  ~ES3Buffer() override;

  void PrepareAllocation() override;

  ref_ptr<MemoryHeap> memory_heap() const override;

  GLenum target() const { return target_; }
  GLuint buffer_id() const { return buffer_id_; }

  void ReadDataRegionsOnQueue(absl::Span<const ReadBufferRegion> data_regions)
      XRTL_REQUIRES_GL_CONTEXT;
  void WriteDataRegionsOnQueue(absl::Span<const WriteBufferRegion> data_regions)
      XRTL_REQUIRES_GL_CONTEXT;

  void InvalidateMappedMemory(size_t byte_offset, size_t byte_length) override;

  void FlushMappedMemory(size_t byte_offset, size_t byte_length) override;

 private:
  void Release() override;
  bool AllocateOnQueue() override;
  void DeallocateOnQueue() override;

  bool MapMemory(MemoryAccess memory_access, size_t* byte_offset,
                 size_t* byte_length, void** out_data) override;
  void UnmapMemory(size_t byte_offset, size_t byte_length, void* data) override;

  ES3ObjectLifetimeQueue* queue_;
  ref_ptr<MemoryHeap> memory_heap_;

  GLenum target_ = GL_COPY_READ_BUFFER;
  GLuint buffer_id_ = 0;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_BUFFER_H_
