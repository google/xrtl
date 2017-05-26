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

#include "xrtl/gfx/util/memory_command_buffer.h"

#include <utility>

namespace xrtl {
namespace gfx {
namespace util {

MemoryCommandBuffer::MemoryCommandBuffer() = default;

MemoryCommandBuffer::~MemoryCommandBuffer() = default;

void MemoryCommandBuffer::Reset() {
  ReleaseDependencies();
  arena_.Reset();
  writer_ = MemoryCommandBufferWriter{&arena_};
  queue_mask_ = OperationQueueMask::kNone;
}

MemoryCommandBufferReader MemoryCommandBuffer::GetReader() const {
  return MemoryCommandBufferReader{writer_.first_packet()};
}

TransferCommandEncoderPtr MemoryCommandBuffer::BeginTransferCommands() {
  queue_mask_ |= OperationQueueMask::kTransfer;
  writer_.WriteCommand(CommandType::kBeginTransferCommands,
                       BeginTransferCommandsCommand{});
  return {&transfer_encoder_, [](TransferCommandEncoder* encoder) {
            encoder->command_buffer()->EndTransferCommands(
                {encoder, [](TransferCommandEncoder*) {}});
          }};
}

void MemoryCommandBuffer::EndTransferCommands(
    TransferCommandEncoderPtr encoder) {
  writer_.WriteCommand(CommandType::kEndTransferCommands,
                       EndTransferCommandsCommand{});
  encoder.release();
}

ComputeCommandEncoderPtr MemoryCommandBuffer::BeginComputeCommands() {
  queue_mask_ |= OperationQueueMask::kCompute;
  writer_.WriteCommand(CommandType::kBeginComputeCommands,
                       BeginComputeCommandsCommand{});
  return {&compute_encoder_, [](ComputeCommandEncoder* encoder) {
            encoder->command_buffer()->EndComputeCommands(
                {encoder, [](ComputeCommandEncoder*) {}});
          }};
}

void MemoryCommandBuffer::EndComputeCommands(ComputeCommandEncoderPtr encoder) {
  writer_.WriteCommand(CommandType::kEndComputeCommands,
                       EndComputeCommandsCommand{});
  encoder.release();
}

RenderCommandEncoderPtr MemoryCommandBuffer::BeginRenderCommands() {
  queue_mask_ |= OperationQueueMask::kRender;
  writer_.WriteCommand(CommandType::kBeginRenderCommands,
                       BeginRenderCommandsCommand{});
  return {&render_encoder_, [](RenderCommandEncoder* encoder) {
            encoder->command_buffer()->EndRenderCommands(
                {encoder, [](RenderCommandEncoder*) {}});
          }};
}

void MemoryCommandBuffer::EndRenderCommands(RenderCommandEncoderPtr encoder) {
  writer_.WriteCommand(CommandType::kEndRenderCommands,
                       EndRenderCommandsCommand{});
  encoder.release();
}

RenderPassCommandEncoderPtr MemoryCommandBuffer::BeginRenderPass(
    ref_ptr<RenderPass> render_pass, ref_ptr<Framebuffer> framebuffer,
    ArrayView<ClearColor> clear_colors) {
  queue_mask_ |= OperationQueueMask::kRender;
  AttachDependency(render_pass);
  AttachDependency(framebuffer);
  writer_.WriteCommand(
      CommandType::kBeginRenderPass,
      BeginRenderPassCommand{render_pass.get(), framebuffer.get(),
                             clear_colors.size()});
  writer_.WriteArray(clear_colors);
  return {&render_pass_encoder_, [](RenderPassCommandEncoder* encoder) {
            encoder->command_buffer()->EndRenderPass(
                {encoder, [](RenderPassCommandEncoder*) {}});
          }};
}

void MemoryCommandBuffer::EndRenderPass(RenderPassCommandEncoderPtr encoder) {
  writer_.WriteCommand(CommandType::kEndRenderPass, EndRenderPassCommand{});
  encoder.release();
}

}  // namespace util
}  // namespace gfx
}  // namespace xrtl
