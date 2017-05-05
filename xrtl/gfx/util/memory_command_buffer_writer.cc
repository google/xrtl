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

#include "xrtl/gfx/util/memory_command_buffer_writer.h"

#include "xrtl/base/logging.h"

namespace xrtl {
namespace gfx {
namespace util {

uint8_t* MemoryCommandBufferWriter::AllocateData(size_t length) {
  // Check bytes remaining in packet.
  size_t max_packet_data_length = arena_->block_size() - sizeof(PacketHeader);
  DCHECK_LE(length, max_packet_data_length);
  if (length > packet_bytes_remaining_) {
    // Packet full - allocate another.
    PacketHeader* previous_header = current_packet_header_;
    current_packet_header_ = reinterpret_cast<PacketHeader*>(
        arena_->AllocateBytes(arena_->block_size()));
    current_packet_header_->packet_length = 0;
    current_packet_header_->next_packet = nullptr;
    packet_bytes_remaining_ = max_packet_data_length;
    if (previous_header) {
      previous_header->next_packet = current_packet_header_;
    }
    if (!first_packet_header_) {
      first_packet_header_ = current_packet_header_;
    }
  }

  // Slice off the next bytes in the packet.
  uint8_t* data = reinterpret_cast<uint8_t*>(current_packet_header_) +
                  sizeof(PacketHeader) + current_packet_header_->packet_length;
  current_packet_header_->packet_length += length;
  packet_bytes_remaining_ -= length;
  return data;
}

void MemoryCommandBufferWriter::WriteCommand(CommandType command_type,
                                             const void* command_data,
                                             size_t command_data_length) {
  uint8_t* buffer = AllocateData(sizeof(CommandHeader) + command_data_length);
  CommandHeader* command_header = reinterpret_cast<CommandHeader*>(buffer);
  command_header->command_type = command_type;
  std::memcpy(buffer + sizeof(CommandHeader), command_data,
              command_data_length);
}

void MemoryCommandBufferWriter::WriteData(const void* data,
                                          size_t data_length) {
  if (!data_length) {
    return;
  }
  uint8_t* buffer = AllocateData(data_length);
  std::memcpy(buffer, data, data_length);
}

}  // namespace util
}  // namespace gfx
}  // namespace xrtl
