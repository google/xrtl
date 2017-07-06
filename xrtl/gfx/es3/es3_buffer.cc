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

#include "xrtl/gfx/es3/es3_buffer.h"

#include <utility>

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Buffer::ES3Buffer(ref_ptr<ES3PlatformContext> platform_context,
                     size_t allocation_size, Usage usage_mask)
    : Buffer(allocation_size, usage_mask),
      platform_context_(std::move(platform_context)) {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));

  // TODO(benvanik): pool ID allocation.
  glGenBuffers(1, &buffer_id_);

  // The GL spec allows buffers to be rebound to any target but warns that the
  // implementation may optimize based on the target it was first bound to.
  // For this reason we try to prioritize certain targets when the usage mask
  // specifies them.
  if (any(usage_mask & Buffer::Usage::kIndirectBuffer)) {
    target_ = GL_DRAW_INDIRECT_BUFFER;
  } else if (any(usage_mask & Buffer::Usage::kVertexBuffer)) {
    target_ = GL_ARRAY_BUFFER;
  } else if (any(usage_mask & Buffer::Usage::kIndexBuffer)) {
    target_ = GL_ELEMENT_ARRAY_BUFFER;
  } else if (any(usage_mask & Buffer::Usage::kUniformBuffer)) {
    target_ = GL_UNIFORM_BUFFER;
  } else if (any(usage_mask & Buffer::Usage::kStorageBuffer)) {
    target_ = GL_SHADER_STORAGE_BUFFER;
  } else {
    target_ = GL_COPY_READ_BUFFER;
  }

  // TODO(benvanik): better usage mask. This can make a big difference on some
  //                 implementations (like WebGL).
  GLenum usage = GL_STREAM_DRAW;

  glBindBuffer(target_, buffer_id_);
  glBufferData(target_, allocation_size, nullptr, usage);
  glBindBuffer(target_, 0);
}

ES3Buffer::~ES3Buffer() {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));
  glDeleteBuffers(1, &buffer_id_);
}

bool ES3Buffer::ReadData(size_t source_offset, void* data, size_t data_length) {
  DCHECK_LE(source_offset + data_length, allocation_size());
  // TODO(benvanik): buffer.
  return false;
}

bool ES3Buffer::WriteData(size_t target_offset, const void* data,
                          size_t data_length) {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));
  DCHECK_LE(target_offset + data_length, allocation_size());
  glBindBuffer(target_, buffer_id_);
  glBufferSubData(target_, target_offset, data_length, data);
  glBindBuffer(target_, 0);
  return true;
}

void ES3Buffer::InvalidateMappedMemory(size_t byte_offset, size_t byte_length) {
  // TODO(benvanik): buffer.
}

void ES3Buffer::FlushMappedMemory(size_t byte_offset, size_t byte_length) {
  // TODO(benvanik): buffer.
}

bool ES3Buffer::MapMemory(size_t* byte_offset, size_t* byte_length,
                          void** out_data) {
  // TODO(benvanik): buffer.
  return false;
}

void ES3Buffer::UnmapMemory(size_t byte_offset, size_t byte_length,
                            void* data) {
  // TODO(benvanik): buffer.
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
