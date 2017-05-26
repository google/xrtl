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

#include "xrtl/gfx/es3/es3_command_buffer.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3CommandBuffer::ES3CommandBuffer() = default;

ES3CommandBuffer::~ES3CommandBuffer() { Reset(); }

void ES3CommandBuffer::PrepareState() {
  // TODO(benvanik): other state as required.
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, 16 * 1024, 16 * 1024);
}

void ES3CommandBuffer::Reset() {
  transfer_encoder_ = ES3TransferCommandEncoder(this);
  compute_encoder_ = ES3ComputeCommandEncoder(this);
  render_encoder_ = ES3RenderCommandEncoder(this);
  render_pass_encoder_ = ES3RenderPassCommandEncoder(this);
}

TransferCommandEncoderPtr ES3CommandBuffer::BeginTransferCommands() {
  // unique_ptr wrapper so that we implicitly end the command encoding when it
  // goes out of scope. It's safe if the user has already ended it explicitly.
  return {&transfer_encoder_, [](TransferCommandEncoder* encoder) {
            encoder->command_buffer()->EndTransferCommands(
                {encoder, [](TransferCommandEncoder*) {}});
          }};
}

void ES3CommandBuffer::EndTransferCommands(TransferCommandEncoderPtr encoder) {
  encoder.release();
}

ComputeCommandEncoderPtr ES3CommandBuffer::BeginComputeCommands() {
  // unique_ptr wrapper so that we implicitly end the command encoding when it
  // goes out of scope. It's safe if the user has already ended it explicitly.
  return {&compute_encoder_, [](ComputeCommandEncoder* encoder) {
            encoder->command_buffer()->EndComputeCommands(
                {encoder, [](ComputeCommandEncoder*) {}});
          }};
}

void ES3CommandBuffer::EndComputeCommands(ComputeCommandEncoderPtr encoder) {
  encoder.release();
}

RenderCommandEncoderPtr ES3CommandBuffer::BeginRenderCommands() {
  // unique_ptr wrapper so that we implicitly end the command encoding when it
  // goes out of scope. It's safe if the user has already ended it explicitly.
  return {&render_encoder_, [](RenderCommandEncoder* encoder) {
            encoder->command_buffer()->EndRenderCommands(
                {encoder, [](RenderCommandEncoder*) {}});
          }};
}

void ES3CommandBuffer::EndRenderCommands(RenderCommandEncoderPtr encoder) {
  encoder.release();
}

RenderPassCommandEncoderPtr ES3CommandBuffer::BeginRenderPass(
    ref_ptr<RenderPass> render_pass, ref_ptr<Framebuffer> framebuffer,
    ArrayView<ClearColor> clear_colors) {
  // unique_ptr wrapper so that we implicitly end the command encoding when it
  // goes out of scope. It's safe if the user has already ended it explicitly.
  render_pass_encoder_.BeginRenderPass(render_pass, framebuffer, clear_colors);
  return {&render_pass_encoder_, [](RenderPassCommandEncoder* encoder) {
            encoder->command_buffer()->EndRenderPass(
                {encoder, [](RenderPassCommandEncoder* encoder) {
                   reinterpret_cast<ES3RenderPassCommandEncoder*>(encoder)
                       ->EndRenderPass();
                 }});
          }};
}

void ES3CommandBuffer::EndRenderPass(RenderPassCommandEncoderPtr encoder) {
  if (encoder) {
    render_pass_encoder_.EndRenderPass();
  }
  encoder.release();
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
