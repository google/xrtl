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

#ifndef XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_READER_H_
#define XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_READER_H_

#include "absl/types/span.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/util/memory_commands.h"

namespace xrtl {
namespace gfx {
namespace util {

// Reads data from a command buffer in the form written by
// MemoryCommandBufferWriter.
class MemoryCommandBufferReader {
 public:
  explicit MemoryCommandBufferReader(const PacketHeader* packet)
      : current_packet_(packet) {}

  // Returns true if the reader is empty and there is no more data remaining in
  // the command buffer.
  bool empty() const;

  // Peeks at a command header in the command buffer.
  // Returns nullptr if there is no valid command or the buffer is empty.
  const CommandHeader* PeekCommandHeader();

  // Reads a command struct from the command buffer.
  template <typename T>
  const T& ReadCommand(const CommandHeader* command_header) {
    packet_offset_ += sizeof(CommandHeader) + sizeof(T);
    return *reinterpret_cast<const T*>(
        reinterpret_cast<const uint8_t*>(command_header) +
        sizeof(CommandHeader));
  }

  // Reads a raw data blob from the command buffer.
  const void* ReadData(size_t data_length);

  // Reads an array of primitives/structs from the command buffer.
  template <typename T>
  absl::Span<const T> ReadArray(size_t value_count) {
    if (!value_count) {
      return {};
    }
    return absl::Span<const T>(
        reinterpret_cast<const T*>(ReadData(value_count * sizeof(T))),
        value_count);
  }

  // Reads an array of reference counted objects from the command buffer.
  // The reference counts will not be adjusted.
  template <typename T>
  absl::Span<const ref_ptr<T>> ReadRefPtrArray(size_t value_count) {
    if (!value_count) {
      return {};
    }
    // NOTE: this is safe only because ref_ptr is *just* a pointer.
    // This does not adjust references and the caller - if it copies the
    // ref_ptrs - will need to ensure the references remain valid until they
    // are adjusted.
    return absl::Span<const ref_ptr<T>>(
        const_cast<ref_ptr<T>*>(reinterpret_cast<const ref_ptr<T>*>(
            ReadData(value_count * sizeof(T*)))),
        value_count);
  }

 private:
  // Advances to the next packet in the stream if the current packet is out of
  // data.
  bool AdvancePacketIfNeeded(size_t required_length);

  const PacketHeader* current_packet_ = nullptr;
  size_t packet_offset_ = 0;
};

}  // namespace util
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_READER_H_
