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

#ifndef XRTL_GFX_ES3_ES3_COMMAND_BUFFER_H_
#define XRTL_GFX_ES3_ES3_COMMAND_BUFFER_H_

#include "xrtl/gfx/command_buffer.h"
#include "xrtl/gfx/es3/es3_command_encoder.h"
#include "xrtl/gfx/es3/es3_common.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// Concrete command buffer implementation for GL.
// This is used to execute commands against the current GL context and are
// called by the MemoryCommandBuffer implementation decoding a previously
// generated command buffer. To record a new command buffer MemoryCommandBuffer
// is used instead of this.
class ES3CommandBuffer : public CommandBuffer {
 public:
  ES3CommandBuffer();
  ~ES3CommandBuffer() override;

  // Prepares GL state for rendering.
  // This should be called each time the command buffer is executed to ensure
  // GL state is reset to its default values.
  void PrepareState();

  // Resets all command buffer tracking.
  // This will drop any retained resources and prepare the command buffer for
  // more execution.
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

 private:
  ES3TransferCommandEncoder transfer_encoder_{this};
  ES3ComputeCommandEncoder compute_encoder_{this};
  ES3RenderCommandEncoder render_encoder_{this};
  ES3RenderPassCommandEncoder render_pass_encoder_{this};
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_COMMAND_BUFFER_H_
