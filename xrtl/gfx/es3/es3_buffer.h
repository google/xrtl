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
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3Buffer : public Buffer {
 public:
  ES3Buffer(ref_ptr<ES3PlatformContext> platform_context,
            ref_ptr<MemoryHeap> memory_heap, size_t allocation_size,
            Usage usage_mask);
  ~ES3Buffer() override;

  ref_ptr<MemoryHeap> memory_heap() const override;

  GLenum target() const { return target_; }
  GLuint buffer_id() const { return buffer_id_; }

  bool ReadData(size_t source_offset, void* data, size_t data_length) override;

  bool WriteData(size_t target_offset, const void* data,
                 size_t data_length) override;

  void InvalidateMappedMemory(size_t byte_offset, size_t byte_length) override;

  void FlushMappedMemory(size_t byte_offset, size_t byte_length) override;

  void Release() override;

 private:
  bool MapMemory(MemoryAccess memory_access, size_t* byte_offset,
                 size_t* byte_length, void** out_data) override;
  void UnmapMemory(size_t byte_offset, size_t byte_length, void* data) override;

  ref_ptr<ES3PlatformContext> platform_context_;
  ref_ptr<MemoryHeap> memory_heap_;

  GLenum target_ = GL_COPY_READ_BUFFER;
  GLuint buffer_id_ = 0;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_BUFFER_H_
