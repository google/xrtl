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

#ifndef XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_WRITER_H_
#define XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_WRITER_H_

#include "xrtl/base/arena.h"
#include "xrtl/base/array_view.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/util/memory_commands.h"

namespace xrtl {
namespace gfx {
namespace util {

// Writes data to a command buffer in a form that MemoryCommandBufferReader can
// consume.
class MemoryCommandBufferWriter {
 public:
  explicit MemoryCommandBufferWriter(Arena* arena) : arena_(arena) {}

  // Returns a pointer to the first packet header, if any packets have been
  // written.
  const PacketHeader* first_packet() const { return first_packet_header_; }

  // Writes a command type and struct to the buffer.
  // Additional data may follow.
  void WriteCommand(CommandType command_type, const void* command_data,
                    size_t command_data_length);
  template <typename T>
  void WriteCommand(CommandType command_type, const T& command_data) {
    WriteCommand(command_type, &command_data, sizeof(T));
  }

  // Writes a raw data blob to the command buffer.
  // This will have no header and should only be used to attach additional data
  // following a WriteCommand.
  void WriteData(const void* data, size_t data_length);

  // Writes an array of primitives/structs to the command buffer.
  template <typename T>
  void WriteArray(ArrayView<T> values) {
    if (values.empty()) {
      return;
    }
    WriteData(values.data(), values.size() * sizeof(T));
  }

  // Writes an array of reference counted objects to the command buffer.
  // The reference counts will not be adjusted.
  template <typename T>
  void WriteArray(ArrayView<ref_ptr<T>> values) {
    if (values.empty()) {
      return;
    }
    // NOTE: this is safe only because ref_ptr is *just* a pointer.
    WriteData(reinterpret_cast<const T* const*>(values.data()),
              values.size() * sizeof(T*));
  }

 private:
  // Allocates at least the given amount of data from the command buffer.
  uint8_t* AllocateData(size_t length);

  Arena* arena_ = nullptr;
  PacketHeader* first_packet_header_ = nullptr;
  PacketHeader* current_packet_header_ = nullptr;
  size_t packet_bytes_remaining_ = 0;
};

}  // namespace util
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_WRITER_H_
