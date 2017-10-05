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

#ifndef XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_H_
#define XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_H_

#include "xrtl/base/arena.h"
#include "xrtl/gfx/command_buffer.h"
#include "xrtl/gfx/util/memory_command_buffer_reader.h"
#include "xrtl/gfx/util/memory_command_buffer_writer.h"
#include "xrtl/gfx/util/memory_command_encoder.h"

namespace xrtl {
namespace gfx {
namespace util {

// A command buffer implementation that encodes commands to a heap memory
// buffer. This is used by backends that do not natively support command buffer
// recording.
class MemoryCommandBuffer : public CommandBuffer {
 public:
  MemoryCommandBuffer();
  ~MemoryCommandBuffer() override;

  // Returns a reader to the start of the packet stream.
  MemoryCommandBufferReader GetReader() const;

  // Resets the command buffer without deallocating memory.
  void Reset();

  TransferCommandEncoderPtr BeginTransferCommands() override;
  void EndTransferCommands(TransferCommandEncoderPtr encoder) override;

  ComputeCommandEncoderPtr BeginComputeCommands() override;
  void EndComputeCommands(ComputeCommandEncoderPtr encoder) override;

  RenderCommandEncoderPtr BeginRenderCommands() override;
  void EndRenderCommands(RenderCommandEncoderPtr encoder) override;

  RenderPassCommandEncoderPtr BeginRenderPass(
      ref_ptr<RenderPass> render_pass, ref_ptr<Framebuffer> framebuffer,
      absl::Span<const ClearColor> clear_colors) override;
  void EndRenderPass(RenderPassCommandEncoderPtr encoder) override;

 protected:
  Arena arena_{kMaxCommandSize};
  MemoryCommandBufferWriter writer_{&arena_};

  MemoryTransferCommandEncoder transfer_encoder_{this, &writer_};
  MemoryComputeCommandEncoder compute_encoder_{this, &writer_};
  MemoryRenderCommandEncoder render_encoder_{this, &writer_};
  MemoryRenderPassCommandEncoder render_pass_encoder_{this, &writer_};
};

}  // namespace util
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_UTIL_MEMORY_COMMAND_BUFFER_H_
