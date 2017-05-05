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

#include "xrtl/gfx/util/memory_command_buffer_reader.h"

namespace xrtl {
namespace gfx {
namespace util {

bool MemoryCommandBufferReader::empty() const {
  return !current_packet_ ||
         (current_packet_->packet_length - packet_offset_ == 0);
}

bool MemoryCommandBufferReader::AdvancePacketIfNeeded(size_t required_length) {
  if (!current_packet_) {
    // No more packets.
    return false;
  }

  if (current_packet_->packet_length - packet_offset_ >= required_length) {
    // Current packet contains enough data to satisfy the request.
    return true;
  }

  // Packet does not contain enough data. Advance to the next one.
  current_packet_ = current_packet_->next_packet;
  packet_offset_ = 0;

  // If that was the last packet, we're done.
  if (!current_packet_) {
    return false;
  }

  // Ensure the new packet has enough data. If not, the stream is invalid.
  DCHECK_LE(required_length, current_packet_->packet_length);

  return true;
}

const CommandHeader* MemoryCommandBufferReader::PeekCommandHeader() {
  if (!AdvancePacketIfNeeded(sizeof(CommandHeader))) {
    return nullptr;
  }

  // Get a pointer to the command data header.
  // We know that the entire header + command data is valid in the current
  // packet due to the way we allocate the slab in the writer. This allows
  // ReadCommand to be simple pointer math instead of a full packet check.
  return reinterpret_cast<const CommandHeader*>(
      (reinterpret_cast<const uint8_t*>(current_packet_)) +
      sizeof(PacketHeader) + packet_offset_);
}

const void* MemoryCommandBufferReader::ReadData(size_t data_length) {
  if (!data_length) {
    return nullptr;
  }
  if (!AdvancePacketIfNeeded(data_length)) {
    return nullptr;
  }

  auto data_ptr = reinterpret_cast<const uint8_t*>(current_packet_) +
                  sizeof(PacketHeader) + packet_offset_;
  packet_offset_ += data_length;
  return data_ptr;
}

}  // namespace util
}  // namespace gfx
}  // namespace xrtl
