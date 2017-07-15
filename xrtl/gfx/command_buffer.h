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

#ifndef XRTL_GFX_COMMAND_BUFFER_H_
#define XRTL_GFX_COMMAND_BUFFER_H_

#include <vector>

#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/command_encoder.h"
#include "xrtl/gfx/framebuffer.h"
#include "xrtl/gfx/render_pass.h"

namespace xrtl {
namespace gfx {

// A bitmask indicating the kind of queue operations will require.
enum class OperationQueueMask : uint32_t {
  kNone = 0,
  // A queue supporting render operations (such as Draw*).
  kRender = 1 << 0,
  // A queue supporting compute operations (such as Dispatch*).
  kCompute = 1 << 1,
  // A queue supporting transfer operations (such as CopyBuffer).
  kTransfer = 1 << 2,
  // A queue supportion presentation operations (via SwapChain).
  kPresent = 1 << 3,
  // A union of all operation queue modes.
  kAll = kRender | kCompute | kTransfer | kPresent,
};
XRTL_BITMASK(OperationQueueMask);

// Transient single-shot command buffer.
// Command buffer lifetime is generally:
//  - Create from Context.
//  - Record with begin/encode/end one or more command encoders.
//  - Submit the command buffer on the Context.
//    (release and recycle)
//
// Commands can be recorded until a command buffer is submitted to the context
// after which time it must not be modified. Multiple command encoders of either
// the same or different types can be began/ended while recording within a
// single CommandBuffer. Note that command buffers that use multiple queues may
// require internal synchronization with barriers or CommandFences.
//
// Usage:
//   // Allocate command buffer for recording.
//   auto command_buffer = context->CreateCommandBuffer();
//   // Record transfer commands preparing buffers.
//   auto transfer_encoder = command_buffer->BeginTransferCommands();
//   transfer_encoder->FillBuffer(...);
//   command_buffer->EndTransferCommands(std::move(transfer_encoder));
//   // Record compute commands that use the buffers.
//   auto compute_encoder = command_buffer->BeginComputeCommands();
//   compute_encoder->Dispatch(...);
//   command_buffer->EndComputeCommands(std::move(compute_encoder));
//   // Submit the command buffer for execution.
//   context->Submit(std::move(command_buffer), signal_fence);
//
class CommandBuffer : public RefObject<CommandBuffer> {
 public:
  virtual ~CommandBuffer();

  // A bitmask indicating on which queue types this command buffer will execute
  // based on the commands that were encoded into it.
  OperationQueueMask queue_mask() const { return queue_mask_; }

  // TODO(benvanik): queries and timestamps.
  // TODO(benvanik): tessellation.

  // Begins encoding transfer commands into the command buffer.
  // All commands encoded with the returned encoder will be written in order to
  // the buffer.
  //
  // Transfer commands will execute on the transfer queue.
  //
  // Only one encoder may be active at a time. When done encoding callers must
  // give the encoder back to the command buffer with EndTransferCommands.
  // Encoders are not thread-safe.
  //
  // Usage:
  //  auto encoder = command_buffer->BeginTransferCommands();
  //  encoder->FillBuffer(...);
  //  command_buffer->EndTransferCommands(std::move(encoder));
  virtual TransferCommandEncoderPtr BeginTransferCommands() = 0;
  virtual void EndTransferCommands(TransferCommandEncoderPtr encoder) = 0;

  // Begins encoding compute commands into the command buffer.
  // All commands encoded with the returned encoder will be written in order to
  // the buffer.
  //
  // Compute commands will execute on the compute and/or transfer queues.
  //
  // Only one encoder may be active at a time. When done encoding callers must
  // give the encoder back to the command buffer with EndComputeCommands.
  // Encoders are not thread-safe.
  //
  // Usage:
  //  auto encoder = command_buffer->BeginComputeCommands();
  //  encoder->Dispatch(...);
  //  command_buffer->EndComputeCommands(std::move(encoder));
  virtual ComputeCommandEncoderPtr BeginComputeCommands() = 0;
  virtual void EndComputeCommands(ComputeCommandEncoderPtr encoder) = 0;

  // Begins encoding render commands into the command buffer.
  // All commands encoded with the returned encoder will be written in order to
  // the buffer.
  //
  // Render commands will execute on the render and/or transfer queues.
  //
  // Only one encoder may be active at a time. When done encoding callers must
  // give the encoder back to the command buffer with EndRenderCommands.
  // Encoders are not thread-safe.
  //
  // Usage:
  //  auto encoder = command_buffer->BeginRenderCommands();
  //  encoder->ResolveImage(...);
  //  command_buffer->EndRenderCommands(std::move(encoder));
  virtual RenderCommandEncoderPtr BeginRenderCommands() = 0;
  virtual void EndRenderCommands(RenderCommandEncoderPtr encoder) = 0;

  // Begins a render pass and encoding commands into the command buffer.
  // All commands encoded with the returned encoder will be written in order to
  // the buffer.
  //
  // The render pass begins in the first defined subpass. Use NextSubPass to
  // advance to the next subpass until all subpasses have been populated.
  //
  // Render pass commands will execute on the render queue.
  //
  // Only one encoder may be active at a time. When done encoding callers must
  // give the encoder back to the command buffer with EndRenderPass.
  // Encoders are not thread-safe.
  //
  // Usage:
  //  auto encoder = command_buffer->BeginRenderPass(render_pass, framebuffer);
  //  encoder->Draw(...);
  //  encoder->NextSubPass();
  //  encoder->Draw(...);
  //  command_buffer->EndRenderPass(std::move(encoder));
  virtual RenderPassCommandEncoderPtr BeginRenderPass(
      ref_ptr<RenderPass> render_pass, ref_ptr<Framebuffer> framebuffer,
      ArrayView<ClearColor> clear_colors) = 0;
  virtual void EndRenderPass(RenderPassCommandEncoderPtr encoder) = 0;

  // Attaches a dependency to the command buffer that will be released when the
  // command buffer has completed executing (or soon thereafter).
  void AttachDependency(void (*release_fn)(void* value_ptr), void* value_ptr);
  void AttachDependencies(void (*release_fn)(void* value_ptr),
                          void* const* value_ptrs, size_t value_count);
  template <typename T>
  void AttachDependency(ref_ptr<T> value) {
    value->AddReference();
    AttachDependency(
        [](void* value_ptr) { static_cast<T*>(value_ptr)->ReleaseReference(); },
        value.get());
  }
  template <typename T>
  void AttachDependencies(ArrayView<ref_ptr<T>> values) {
    for (const auto& value : values) {
      value->AddReference();
    }
    // TODO(benvanik): static asserts so that this is safer.
    AttachDependencies(
        [](void* value_ptr) { static_cast<T*>(value_ptr)->ReleaseReference(); },
        reinterpret_cast<void* const*>(values.data()), values.size());
  }

 protected:
  CommandBuffer();

  // Releases all dependencies held by the command buffer.
  void ReleaseDependencies();

  OperationQueueMask queue_mask_ = OperationQueueMask::kNone;

  // A cache of all attached dependencies.
  struct DependencyEntry {
    void (*release_fn)(void* value_ptr);
    void* value_ptr;
  };
  std::vector<DependencyEntry> dependencies_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_COMMAND_BUFFER_H_
