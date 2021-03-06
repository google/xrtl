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

#include <algorithm>
#include <utility>

#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/memory_heap.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Buffer::ES3Buffer(ES3ObjectLifetimeQueue* queue,
                     ref_ptr<MemoryHeap> memory_heap, size_t allocation_size,
                     Usage usage_mask)
    : Buffer(allocation_size, usage_mask),
      queue_(queue),
      memory_heap_(std::move(memory_heap)) {
}

ES3Buffer::~ES3Buffer() = default;

void ES3Buffer::PrepareAllocation() { queue_->EnqueueObjectAllocation(this); }

void ES3Buffer::Release() {
  memory_heap_->ReleaseBuffer(this);
  queue_->EnqueueObjectDeallocation(this);
}

bool ES3Buffer::AllocateOnQueue() {
  WTF_SCOPE0("ES3Buffer#AllocateOnQueue");
  ES3PlatformContext::CheckHasContextLock();

  // TODO(benvanik): pool ID allocation.
  glGenBuffers(1, &buffer_id_);

  // The GL spec allows buffers to be rebound to any target but warns that the
  // implementation may optimize based on the target it was first bound to.
  // For this reason we try to prioritize certain targets when the usage mask
  // specifies them.
  if (any(usage_mask() & Buffer::Usage::kIndirectBuffer)) {
    target_ = GL_DRAW_INDIRECT_BUFFER;
  } else if (any(usage_mask() & Buffer::Usage::kVertexBuffer)) {
    target_ = GL_ARRAY_BUFFER;
  } else if (any(usage_mask() & Buffer::Usage::kIndexBuffer)) {
    target_ = GL_ELEMENT_ARRAY_BUFFER;
  } else if (any(usage_mask() & Buffer::Usage::kUniformBuffer)) {
    target_ = GL_UNIFORM_BUFFER;
  } else if (any(usage_mask() & Buffer::Usage::kStorageBuffer)) {
    target_ = GL_SHADER_STORAGE_BUFFER;
  } else {
    target_ = GL_COPY_READ_BUFFER;
  }

  // TODO(benvanik): better usage mask. This can make a big difference on some
  //                 implementations (like WebGL).
  GLenum usage = GL_DYNAMIC_DRAW;

  glBindBuffer(target_, buffer_id_);
  glBufferData(target_, allocation_size(), nullptr, usage);
  glBindBuffer(target_, 0);

  return true;
}

void ES3Buffer::DeallocateOnQueue() {
  WTF_SCOPE0("ES3Buffer#DeallocateOnQueue");
  ES3PlatformContext::CheckHasContextLock();
  if (buffer_id_) {
    glDeleteBuffers(1, &buffer_id_);
    buffer_id_ = 0;
  }
}

ref_ptr<MemoryHeap> ES3Buffer::memory_heap() const { return memory_heap_; }

void ES3Buffer::ReadDataRegionsOnQueue(
    absl::Span<const ReadBufferRegion> data_regions) {
  WTF_SCOPE0("ES3Buffer#ReadDataRegionsOnQueue");
  ES3PlatformContext::CheckHasContextLock();

  // Must be mappable.
  bool is_mappable =
      any(memory_heap_->memory_type_mask() & MemoryType::kHostVisible);
  DCHECK(is_mappable);
  if (!is_mappable) {
    LOG(ERROR) << "Attempting to map a non-host-visible memory buffer";
    DCHECK(false);
    return;
  }

  // Find the full range desired for reading so that we don't need to map the
  // entire buffer. If the ranges are very disjoint this will suck, but it's
  // likely cheaper than doing N map/unmaps.
  size_t min_offset = allocation_size() - 1;
  size_t max_offset = 0;
  for (const ReadBufferRegion& data_region : data_regions) {
    min_offset = std::min(min_offset, data_region.source_offset);
    max_offset = std::max(max_offset, data_region.source_offset +
                                          data_region.target_data_length - 1);
  }

  // TODO(benvanik): validate and align offset/length.

  // Map the buffer.
  // TODO(benvanik): find a better way to do this, like APPLE_client_storage.
  glBindBuffer(target_, buffer_id_);
  const uint8_t* buffer_data_ptr =
      reinterpret_cast<const uint8_t*>(glMapBufferRange(
          target_, min_offset, max_offset - min_offset + 1, GL_MAP_READ_BIT));
  if (!buffer_data_ptr) {
    LOG(ERROR) << "Failed to map buffer";
    DCHECK(false);
    return;
  }

  for (const ReadBufferRegion& data_region : data_regions) {
    DCHECK_LE(data_region.source_offset + data_region.target_data_length,
              allocation_size());
    std::memcpy(data_region.target_data,
                buffer_data_ptr + data_region.source_offset - min_offset,
                data_region.target_data_length);
  }

  GLboolean unmapped = glUnmapBuffer(target_);
  glBindBuffer(target_, 0);
  DCHECK_EQ(unmapped, GL_TRUE);
}

void ES3Buffer::WriteDataRegionsOnQueue(
    absl::Span<const WriteBufferRegion> data_regions) {
  WTF_SCOPE0("ES3Buffer#WriteDataRegionsOnQueue");
  ES3PlatformContext::CheckHasContextLock();
  glBindBuffer(target_, buffer_id_);
  for (const WriteBufferRegion& data_region : data_regions) {
    DCHECK_LE(data_region.target_offset + data_region.source_data_length,
              allocation_size());
    glBufferSubData(target_, data_region.target_offset,
                    data_region.source_data_length, data_region.source_data);
  }
  glBindBuffer(target_, 0);
}

bool ES3Buffer::MapMemory(MemoryAccess memory_access, size_t* byte_offset,
                          size_t* byte_length, void** out_data) {
  WTF_SCOPE0("ES3Buffer#MapMemory");
  return queue_->EnqueueObjectCallbackAndWait(this, [this, memory_access,
                                                     byte_offset, byte_length,
                                                     out_data]() {
    WTF_SCOPE0("ES3Buffer#MapMemory:queue");
    ES3PlatformContext::CheckHasContextLock();

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
          // Mapping the entire buffer so we can drop it all. This is most
          // likely identical to invalidating the range but since it's in the
          // spec and I don't trust drivers we'll be explicit.
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
  });
}

void ES3Buffer::UnmapMemory(size_t byte_offset, size_t byte_length,
                            void* data) {
  WTF_SCOPE0("ES3Buffer#UnmapMemory");
  queue_->EnqueueObjectCallbackAndWait(this, [this]() {
    WTF_SCOPE0("ES3Buffer#UnmapMemory:queue");
    ES3PlatformContext::CheckHasContextLock();

    glBindBuffer(target_, buffer_id_);
    GLboolean unmapped = glUnmapBuffer(target_);
    glBindBuffer(target_, 0);

    DCHECK_EQ(unmapped, GL_TRUE);
    if (unmapped == GL_FALSE) {
      LOG(FATAL) << "Buffer corruption while mapped";
    }

    return true;
  });
}

void ES3Buffer::InvalidateMappedMemory(size_t byte_offset, size_t byte_length) {
  // This is a no-op on GL. No issues with not doing it (in theory) - just perf.
}

void ES3Buffer::FlushMappedMemory(size_t byte_offset, size_t byte_length) {
  WTF_SCOPE0("ES3Buffer#FlushMappedMemory");
  queue_->EnqueueObjectCallbackAndWait(
      this, [this, byte_offset, byte_length]() {
        WTF_SCOPE0("ES3Buffer#FlushMappedMemory:queue");
        ES3PlatformContext::CheckHasContextLock();

        // Flushes are ignored with kHostCoherent memory.
        if (any(memory_heap_->memory_type_mask() & MemoryType::kHostCoherent)) {
          return true;
        }

        glBindBuffer(target_, buffer_id_);
        glFlushMappedBufferRange(target_, byte_offset, byte_length);
        glBindBuffer(target_, 0);

        return true;
      });
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
