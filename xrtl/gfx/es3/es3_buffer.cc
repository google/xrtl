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

#include "xrtl/gfx/memory_heap.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Buffer::ES3Buffer(ref_ptr<ES3PlatformContext> platform_context,
                     ref_ptr<MemoryHeap> memory_heap, size_t allocation_size,
                     Usage usage_mask)
    : Buffer(allocation_size, usage_mask),
      platform_context_(std::move(platform_context)),
      memory_heap_(std::move(memory_heap)) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

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
  GLenum usage = GL_DYNAMIC_DRAW;

  glBindBuffer(target_, buffer_id_);
  glBufferData(target_, allocation_size, nullptr, usage);
  glBindBuffer(target_, 0);
}

ES3Buffer::~ES3Buffer() {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
  glDeleteBuffers(1, &buffer_id_);
}

ref_ptr<MemoryHeap> ES3Buffer::memory_heap() const { return memory_heap_; }

void ES3Buffer::Release() { memory_heap_->ReleaseBuffer(this); }

bool ES3Buffer::ReadData(size_t source_offset, void* data, size_t data_length) {
  DCHECK_LE(source_offset + data_length, allocation_size());
  // TODO(benvanik): buffer.
  DCHECK(false);
  return false;
}

bool ES3Buffer::WriteData(size_t target_offset, const void* data,
                          size_t data_length) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
  DCHECK_LE(target_offset + data_length, allocation_size());
  glBindBuffer(target_, buffer_id_);
  glBufferSubData(target_, target_offset, data_length, data);
  glBindBuffer(target_, 0);
  return true;
}

bool ES3Buffer::MapMemory(MemoryAccess memory_access, size_t* byte_offset,
                          size_t* byte_length, void** out_data) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

  *out_data = nullptr;

  // Must be mappable.
  bool is_mappable =
      any(memory_heap_->memory_type_mask() & MemoryType::kHostVisible);
  DCHECK(is_mappable);
  if (!is_mappable) {
    LOG(ERROR) << "Attempting to map a non-host-visible memory buffer";
    return false;
  }

  // TODO(benvanik): validate and align offset/length.

  GLbitfield access = 0;
  switch (memory_access) {
    case MemoryAccess::kRead:
      access = GL_MAP_READ_BIT;
      break;
    case MemoryAccess::kWrite:
      access = GL_MAP_WRITE_BIT;
      break;
    case MemoryAccess::kWriteDiscard:
      access = GL_MAP_WRITE_BIT;
      if (*byte_offset == 0 && *byte_length == allocation_size()) {
        // Mapping the entire buffer so we can drop it all. This is most likely
        // identical to invalidating the range but since it's in the spec and I
        // don't trust drivers we'll be explicit.
        access |= GL_MAP_INVALIDATE_BUFFER_BIT;
      } else {
        access |= GL_MAP_INVALIDATE_RANGE_BIT;
      }
      break;
  }

  if (access & GL_MAP_WRITE_BIT) {
    // Non-host-coherent memory requires explicit flushes.
    if (!any(memory_heap_->memory_type_mask() & MemoryType::kHostCoherent)) {
      access |= GL_MAP_UNSYNCHRONIZED_BIT;
      access |= GL_MAP_FLUSH_EXPLICIT_BIT;
    }
  }

  // TODO(benvanik): see if we can set GL_MAP_UNSYNCHRONIZED_BIT.

  glBindBuffer(target_, buffer_id_);
  void* data = glMapBufferRange(target_, *byte_offset, *byte_length, access);
  glBindBuffer(target_, 0);
  if (!data) {
    LOG(ERROR) << "Failed to map buffer";
    return false;
  }
  *out_data = data;

  return true;
}

void ES3Buffer::UnmapMemory(size_t byte_offset, size_t byte_length,
                            void* data) {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

  glBindBuffer(target_, buffer_id_);
  GLboolean unmapped = glUnmapBuffer(target_);
  glBindBuffer(target_, 0);

  DCHECK_EQ(unmapped, GL_TRUE);
  if (unmapped == GL_FALSE) {
    LOG(FATAL) << "Buffer corruption while mapped";
  }
}

void ES3Buffer::InvalidateMappedMemory(size_t byte_offset, size_t byte_length) {
  // This is a no-op on GL. No issues with not doing it (in theory) - just perf.
}

void ES3Buffer::FlushMappedMemory(size_t byte_offset, size_t byte_length) {
  // Flushes are ignored with kHostCoherent memory.
  if (any(memory_heap_->memory_type_mask() & MemoryType::kHostCoherent)) {
    return;
  }

  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

  glBindBuffer(target_, buffer_id_);
  glFlushMappedBufferRange(target_, byte_offset, byte_length);
  glBindBuffer(target_, 0);
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
