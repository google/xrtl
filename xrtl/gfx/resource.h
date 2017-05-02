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

class Resource;

// A memory mapping RAII object.
// The mapping will stay active until this is reset.
template <typename T>
class MappedMemory {
 public:
  typedef const T* MappedMemory<T>::*unspecified_bool_type;

  MappedMemory() = default;

  MappedMemory(ref_ptr<Resource> resource, size_t byte_offset,
               size_t byte_length, size_t size, T* data)
      : resource_(resource),
        byte_offset_(byte_offset),
        byte_length_(byte_length),
        size_(size),
        data_(data) {}

  ~MappedMemory() { reset(); }

  // The resource that this mapping references.
  ref_ptr<Resource> resource() const { return resource_; }
  // Offset, in bytes, into the resource allocation.
  size_t byte_offset() const noexcept { return byte_offset_; }
  // Length, in bytes, of the resource mapping.
  // This may be larger than the originally requested length due to alignment.
  size_t byte_length() const noexcept { return byte_length_; }

  // True if the mapping is empty.
  bool empty() const noexcept { return size_ == 0; }
  // The size of the mapping as requested in elements.
  size_t size() const noexcept { return size_; }
  // Returns a pointer to the mapped memory.
  // This will be nullptr if the mapping failed.
  const T* data() const noexcept { return data_; }

  // Accesses an element in the mapped memory.
  // Must be called with a valid index in [0, size()).
  const T& operator[](size_t i) const noexcept { return data_[i]; }

  // Supports boolean expression evaluation.
  operator unspecified_bool_type() const {
    return byte_length_ > 0 ? &MappedMemory::data_ : 0;
  }
  // Supports unary expression evaluation.
  bool operator!() const { return byte_length_ == 0; }

  // Invalidates all mapping non-coherent memory from the host caches.
  void Invalidate();
  // Invalidates a range of non-coherent elements from the host caches.
  void Invalidate(size_t element_offset, size_t element_length);

  // Flushes all mapping non-coherent memory from the host caches.
  void Flush();
  // Flushes a range of non-coherent elements from the host caches.
  void Flush(size_t element_offset, size_t element_length);

  // Unmaps the mapped memory.
  // The memory will not be implicitly flushed when unmapping.
  void reset();

 private:
  ref_ptr<Resource> resource_;
  size_t byte_offset_ = 0;
  size_t byte_length_ = 0;
  size_t size_ = 0;
  T* data_ = nullptr;
};

// Base type for allocated resources.
class Resource : public RefObject<Resource> {
 public:
  virtual ~Resource() = default;

  // Size of the resource memory allocation in bytes.
  // This may be rounded up from the originally requested size or the ideal
  // size for the resource based on device restrictions.
  size_t allocation_size() const { return allocation_size_; }

  // Reads a block of data from the resource at the given offset.
  //
  // Returns false if the read could not be performed; either the bounds are
  // out of range or the memory type does not support reading in this way.
  virtual bool ReadData(size_t source_offset, void* data,
                        size_t data_length) = 0;

  // Writes a block of data into the resource at the given offset.
  //
  // Returns false if the write could not be performed; either the bounds are
  // out of range or the memory type does not support writing in this way.
  virtual bool WriteData(size_t target_offset, const void* data,
                         size_t data_length) = 0;

  // Maps the resource memory for direct access from the host.
  // This requires that the resource was allocated with
  // MemoryType::kHostVisible.
  //
  // If MemoryType::kHostCoherent was not specified the explicit
  // InvalidateMappedMemory and FlushMappedMemory must be used to control
  // visibility of the data on the device. If MemoryType::kHostCached
  // is not set callers should not attempt to read from the mapped memory, as
  // doing so may produce undefined results and/or ultra slow reads.
  //
  // This allows mapping the memory as a C++ type. Care must be taken to ensure
  // the data layout in C++ matches the expected data layout in the shaders that
  // consume this data. For simple primitives like uint8_t or float this is
  // usually not a problem, however struct packing may have many restrictions.
  //
  // The returned mapping should be unmapped when it is no longer required.
  // Devices may conditionally support persistent mappings that can be left
  // mapped for longer, however doing so has a cost. Unmapping does not
  // implicitly flush.
  //
  // Returns nullptr if the memory could not be mapped due to mapping
  // exhaustion, invalid arguments, or unsupported memory types.
  //
  // Usage:
  //  MappedMemory memory = buffer->MapMemory<MyStruct>();
  //  memory[5].foo = 3;
  //  std::memcpy(memory.data(), source_data, memory.size());
  //  memory.reset();
  template <typename T>
  MappedMemory<T> MapMemory(size_t element_offset, size_t element_length) {
    size_t byte_offset = element_offset * sizeof(T);
    size_t byte_length = element_length * sizeof(T);
    void* data = nullptr;
    if (!MapMemory(&byte_offset, &byte_length, &data)) {
      return {};
    }
    return {this, byte_offset, byte_length, element_length, data};
  }
  template <typename T>
  MappedMemory<T> MapMemory() {
    return MapMemory<T>(0, allocation_size() / sizeof(T));
  }

  // Invalidates ranges of non-coherent memory from the host caches.
  // Use this before reading from non-coherent memory.
  // This guarantees that device writes to the memory ranges provided are
  // visible on the host.
  // This is only required for memory types without kHostCoherent set.
  virtual void InvalidateMappedMemory(size_t byte_offset,
                                      size_t byte_length) = 0;
  void InvalidateMappedMemory() {
    InvalidateMappedMemory(0, allocation_size());
  }

  // Flushes ranges of non-coherent memory from the host caches.
  // Use this after writing to non-coherent memory.
  // This guarantees that host writes to the memory ranges provided are made
  // available for device access.
  // This is only required for memory types without kHostCoherent set.
  virtual void FlushMappedMemory(size_t byte_offset, size_t byte_length) = 0;
  void FlushMappedMemory() { FlushMappedMemory(0, allocation_size()); }

 protected:
  template <typename T>
  friend class MappedMemory;

  explicit Resource(size_t allocation_size)
      : allocation_size_(allocation_size) {}

  // Maps memory directly.
  // The byte offset and byte length may be adjusted for device alignment.
  // The output data pointer will be properly aligned to the start of the data.
  // Returns false if the memory could not be mapped.
  virtual bool MapMemory(size_t* byte_offset, size_t* byte_length,
                         void** out_data) = 0;

  // Unmaps previously mapped memory.
  virtual void UnmapMemory(size_t byte_offset, size_t byte_length,
                           void* data) = 0;

  size_t allocation_size_ = 0;
};

template <typename T>
void MappedMemory<T>::Invalidate() {
  if (resource_) {
    resource_->InvalidateMappedMemory(byte_offset_, byte_length_);
  }
}

template <typename T>
void MappedMemory<T>::Invalidate(size_t element_offset, size_t element_length) {
  if (resource_) {
    resource_->InvalidateMappedMemory(byte_offset_ + element_offset * sizeof(T),
                                      element_length * sizeof(T));
  }
}

template <typename T>
void MappedMemory<T>::Flush() {
  if (resource_) {
    resource_->FlushMappedMemory(byte_offset_, byte_length_);
  }
}

template <typename T>
void MappedMemory<T>::Flush(size_t element_offset, size_t element_length) {
  if (resource_) {
    resource_->FlushMappedMemory(byte_offset_ + element_offset * sizeof(T),
                                 element_length * sizeof(T));
  }
}

template <typename T>
void MappedMemory<T>::reset() {
  if (resource_) {
    resource_->UnmapMemory(byte_offset_, byte_length_, data_);
    resource_ = nullptr;
    byte_offset_ = byte_length_ = size_ = 0;
    data_ = nullptr;
  }
}

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RESOURCE_H_
