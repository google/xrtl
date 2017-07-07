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

#include "xrtl/gfx/es3/es3_command_encoder.h"

#include "xrtl/base/debugging.h"
#include "xrtl/gfx/es3/es3_buffer.h"
#include "xrtl/gfx/es3/es3_image.h"
#include "xrtl/gfx/es3/es3_pipeline.h"
#include "xrtl/gfx/es3/es3_sampler.h"

namespace xrtl {
namespace gfx {
namespace es3 {

namespace {

constexpr GLenum GetFaceFromFaceMask(StencilFaceFlag face_mask) {
  return (face_mask == StencilFaceFlag::kFrontAndBack)
             ? GL_FRONT_AND_BACK
             : (face_mask == StencilFaceFlag::kFaceFront ? GL_FRONT : GL_BACK);
}

}  // namespace

ES3TransferCommandEncoder::ES3TransferCommandEncoder(
    CommandBuffer* command_buffer)
    : TransferCommandEncoder(command_buffer) {}

ES3TransferCommandEncoder::~ES3TransferCommandEncoder() = default;

void ES3TransferCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  // TODO(benvanik): this.
  LOG(WARNING) << "PipelineBarrier not yet implemented";
}

void ES3TransferCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  // TODO(benvanik): this.
  LOG(WARNING) << "MemoryBarrier not yet implemented";
}

void ES3TransferCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  // TODO(benvanik): this.
  LOG(WARNING) << "BufferBarrier not yet implemented";
}

void ES3TransferCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  // TODO(benvanik): this.
  LOG(WARNING) << "ImageBarrier not yet implemented";
}

void ES3TransferCommandEncoder::FillBuffer(ref_ptr<Buffer> buffer,
                                           size_t offset, size_t length,
                                           uint8_t value) {
  // TODO(benvanik): this.
  LOG(WARNING) << "FillBuffer not yet implemented";
}

void ES3TransferCommandEncoder::UpdateBuffer(ref_ptr<Buffer> base_target_buffer,
                                             size_t target_offset,
                                             const void* source_data,
                                             size_t source_data_length) {
  auto target_buffer = base_target_buffer.As<ES3Buffer>();
  glBindBuffer(target_buffer->target(), target_buffer->buffer_id());
  glBufferSubData(target_buffer->target(), target_offset, source_data_length,
                  source_data);
}

void ES3TransferCommandEncoder::CopyBuffer(
    ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
    ArrayView<CopyBufferRegion> regions) {
  // TODO(benvanik): this.
  LOG(WARNING) << "CopyBuffer not yet implemented";
}

void ES3TransferCommandEncoder::CopyImage(ref_ptr<Image> source_image,
                                          Image::Layout source_image_layout,
                                          ref_ptr<Image> target_image,
                                          Image::Layout target_image_layout,
                                          ArrayView<CopyImageRegion> regions) {
  // TODO(benvanik): this.
  LOG(WARNING) << "CopyImage not yet implemented";
}

void ES3TransferCommandEncoder::CopyBufferToImage(
    ref_ptr<Buffer> source_buffer, ref_ptr<Image> target_image,
    Image::Layout target_image_layout,
    ArrayView<CopyBufferImageRegion> regions) {
  // TODO(benvanik): this.
  LOG(WARNING) << "CopyBufferToImage not yet implemented";
}

void ES3TransferCommandEncoder::CopyImageToBuffer(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Buffer> target_buffer, ArrayView<CopyBufferImageRegion> regions) {
  // TODO(benvanik): this.
  LOG(WARNING) << "CopyImageToBuffer not yet implemented";
}

void ES3TransferCommandEncoder::SetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  // TODO(benvanik): this.
  LOG(WARNING) << "SetFence not yet implemented";
}

void ES3TransferCommandEncoder::ResetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  // TODO(benvanik): this.
  LOG(WARNING) << "ResetFence not yet implemented";
}

void ES3TransferCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  // TODO(benvanik): this.
  LOG(WARNING) << "WaitFences not yet implemented";
}

void ES3TransferCommandEncoder::ClearColorImage(
    ref_ptr<Image> image, Image::Layout image_layout, ClearColor clear_color,
    ArrayView<Image::LayerRange> ranges) {
  // TODO(benvanik): this.
  LOG(WARNING) << "ClearColorImage not yet implemented";
}

ES3ComputeCommandEncoder::ES3ComputeCommandEncoder(
    CommandBuffer* command_buffer)
    : ComputeCommandEncoder(command_buffer), common_encoder_(command_buffer) {}

ES3ComputeCommandEncoder::~ES3ComputeCommandEncoder() = default;

void ES3ComputeCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  common_encoder_.PipelineBarrier(source_stage_mask, target_stage_mask,
                                  dependency_flags);
}

void ES3ComputeCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  common_encoder_.MemoryBarrier(source_stage_mask, target_stage_mask,
                                dependency_flags, source_access_mask,
                                target_access_mask);
}

void ES3ComputeCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  common_encoder_.BufferBarrier(source_stage_mask, target_stage_mask,
                                dependency_flags, source_access_mask,
                                target_access_mask, std::move(buffer), offset,
                                length);
}

void ES3ComputeCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  common_encoder_.ImageBarrier(source_stage_mask, target_stage_mask,
                               dependency_flags, source_access_mask,
                               target_access_mask, source_layout, target_layout,
                               std::move(image), layer_range);
}

void ES3ComputeCommandEncoder::FillBuffer(ref_ptr<Buffer> buffer, size_t offset,
                                          size_t length, uint8_t value) {
  common_encoder_.FillBuffer(std::move(buffer), offset, length, value);
}

void ES3ComputeCommandEncoder::UpdateBuffer(ref_ptr<Buffer> target_buffer,
                                            size_t target_offset,
                                            const void* source_data,
                                            size_t source_data_length) {
  common_encoder_.UpdateBuffer(std::move(target_buffer), target_offset,
                               source_data, source_data_length);
}

void ES3ComputeCommandEncoder::CopyBuffer(ref_ptr<Buffer> source_buffer,
                                          ref_ptr<Buffer> target_buffer,
                                          ArrayView<CopyBufferRegion> regions) {
  common_encoder_.CopyBuffer(std::move(source_buffer), std::move(target_buffer),
                             regions);
}

void ES3ComputeCommandEncoder::CopyImage(ref_ptr<Image> source_image,
                                         Image::Layout source_image_layout,
                                         ref_ptr<Image> target_image,
                                         Image::Layout target_image_layout,
                                         ArrayView<CopyImageRegion> regions) {
  common_encoder_.CopyImage(std::move(source_image), source_image_layout,
                            std::move(target_image), target_image_layout,
                            regions);
}

void ES3ComputeCommandEncoder::CopyBufferToImage(
    ref_ptr<Buffer> source_buffer, ref_ptr<Image> target_image,
    Image::Layout target_image_layout,
    ArrayView<CopyBufferImageRegion> regions) {
  common_encoder_.CopyBufferToImage(std::move(source_buffer),
                                    std::move(target_image),
                                    target_image_layout, regions);
}

void ES3ComputeCommandEncoder::CopyImageToBuffer(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Buffer> target_buffer, ArrayView<CopyBufferImageRegion> regions) {
  common_encoder_.CopyImageToBuffer(std::move(source_image),
                                    source_image_layout,
                                    std::move(target_buffer), regions);
}

void ES3ComputeCommandEncoder::SetFence(ref_ptr<CommandFence> fence,
                                        PipelineStageFlag pipeline_stage_mask) {
  common_encoder_.SetFence(std::move(fence), pipeline_stage_mask);
}

void ES3ComputeCommandEncoder::ResetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  common_encoder_.ResetFence(std::move(fence), pipeline_stage_mask);
}

void ES3ComputeCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  common_encoder_.WaitFences(fences);
}

void ES3ComputeCommandEncoder::ClearColorImage(
    ref_ptr<Image> image, Image::Layout image_layout, ClearColor clear_color,
    ArrayView<Image::LayerRange> ranges) {
  common_encoder_.ClearColorImage(std::move(image), image_layout, clear_color,
                                  ranges);
}

void ES3ComputeCommandEncoder::BindPipeline(ref_ptr<ComputePipeline> pipeline) {
  // TODO(benvanik): this.
  LOG(WARNING) << "BindPipeline not yet implemented";
}

void ES3ComputeCommandEncoder::BindResourceSet(
    ref_ptr<ResourceSet> resource_set, ArrayView<size_t> dynamic_offsets) {
  // TODO(benvanik): this.
  LOG(WARNING) << "BindResourceSet not yet implemented";
}

void ES3ComputeCommandEncoder::PushConstants(
    ref_ptr<PipelineLayout> pipeline_layout, ShaderStageFlag stage_mask,
    size_t offset, const void* data, size_t data_length) {
  // TODO(benvanik): this.
  LOG(WARNING) << "PushConstants not yet implemented";
}

void ES3ComputeCommandEncoder::Dispatch(int group_count_x, int group_count_y,
                                        int group_count_z) {
  // TODO(benvanik): this.
  LOG(WARNING) << "Dispatch not yet implemented";
}

void ES3ComputeCommandEncoder::DispatchIndirect(ref_ptr<Buffer> buffer,
                                                size_t offset) {
  // TODO(benvanik): this.
  LOG(WARNING) << "DispatchIndirect not yet implemented";
}

ES3RenderCommandEncoder::ES3RenderCommandEncoder(CommandBuffer* command_buffer)
    : RenderCommandEncoder(command_buffer), common_encoder_(command_buffer) {}

ES3RenderCommandEncoder::~ES3RenderCommandEncoder() = default;

void ES3RenderCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  common_encoder_.PipelineBarrier(source_stage_mask, target_stage_mask,
                                  dependency_flags);
}

void ES3RenderCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  common_encoder_.MemoryBarrier(source_stage_mask, target_stage_mask,
                                dependency_flags, source_access_mask,
                                target_access_mask);
}

void ES3RenderCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  common_encoder_.BufferBarrier(source_stage_mask, target_stage_mask,
                                dependency_flags, source_access_mask,
                                target_access_mask, std::move(buffer), offset,
                                length);
}

void ES3RenderCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  common_encoder_.ImageBarrier(source_stage_mask, target_stage_mask,
                               dependency_flags, source_access_mask,
                               target_access_mask, source_layout, target_layout,
                               std::move(image), layer_range);
}

void ES3RenderCommandEncoder::FillBuffer(ref_ptr<Buffer> buffer, size_t offset,
                                         size_t length, uint8_t value) {
  common_encoder_.FillBuffer(std::move(buffer), offset, length, value);
}

void ES3RenderCommandEncoder::UpdateBuffer(ref_ptr<Buffer> target_buffer,
                                           size_t target_offset,
                                           const void* source_data,
                                           size_t source_data_length) {
  common_encoder_.UpdateBuffer(std::move(target_buffer), target_offset,
                               source_data, source_data_length);
}

void ES3RenderCommandEncoder::CopyBuffer(ref_ptr<Buffer> source_buffer,
                                         ref_ptr<Buffer> target_buffer,
                                         ArrayView<CopyBufferRegion> regions) {
  common_encoder_.CopyBuffer(std::move(source_buffer), std::move(target_buffer),
                             regions);
}

void ES3RenderCommandEncoder::CopyImage(ref_ptr<Image> source_image,
                                        Image::Layout source_image_layout,
                                        ref_ptr<Image> target_image,
                                        Image::Layout target_image_layout,
                                        ArrayView<CopyImageRegion> regions) {
  common_encoder_.CopyImage(std::move(source_image), source_image_layout,
                            std::move(target_image), target_image_layout,
                            regions);
}

void ES3RenderCommandEncoder::CopyBufferToImage(
    ref_ptr<Buffer> source_buffer, ref_ptr<Image> target_image,
    Image::Layout target_image_layout,
    ArrayView<CopyBufferImageRegion> regions) {
  common_encoder_.CopyBufferToImage(std::move(source_buffer),
                                    std::move(target_image),
                                    target_image_layout, regions);
}

void ES3RenderCommandEncoder::CopyImageToBuffer(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Buffer> target_buffer, ArrayView<CopyBufferImageRegion> regions) {
  common_encoder_.CopyImageToBuffer(std::move(source_image),
                                    source_image_layout,
                                    std::move(target_buffer), regions);
}

void ES3RenderCommandEncoder::SetFence(ref_ptr<CommandFence> fence,
                                       PipelineStageFlag pipeline_stage_mask) {
  common_encoder_.SetFence(std::move(fence), pipeline_stage_mask);
}

void ES3RenderCommandEncoder::ResetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  common_encoder_.ResetFence(std::move(fence), pipeline_stage_mask);
}

void ES3RenderCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  common_encoder_.WaitFences(fences);
}

void ES3RenderCommandEncoder::ClearColorImage(
    ref_ptr<Image> image, Image::Layout image_layout, ClearColor clear_color,
    ArrayView<Image::LayerRange> ranges) {
  common_encoder_.ClearColorImage(std::move(image), image_layout, clear_color,
                                  ranges);
}

void ES3RenderCommandEncoder::ClearDepthStencilImage(
    ref_ptr<Image> image, Image::Layout image_layout, float depth_value,
    uint32_t stencil_value, ArrayView<Image::LayerRange> ranges) {
  // TODO(benvanik): this.
  LOG(WARNING) << "ClearDepthStencilImage not yet implemented";
}

void ES3RenderCommandEncoder::BlitImage(ref_ptr<Image> source_image,
                                        Image::Layout source_image_layout,
                                        ref_ptr<Image> target_image,
                                        Image::Layout target_image_layout,
                                        Sampler::Filter scaling_filter,
                                        ArrayView<BlitImageRegion> regions) {
  // TODO(benvanik): this.
  LOG(WARNING) << "BlitImage not yet implemented";
}

void ES3RenderCommandEncoder::ResolveImage(ref_ptr<Image> source_image,
                                           Image::Layout source_image_layout,
                                           ref_ptr<Image> target_image,
                                           Image::Layout target_image_layout,
                                           ArrayView<CopyImageRegion> regions) {
  // TODO(benvanik): this.
  LOG(WARNING) << "ResolveImage not yet implemented";
}

void ES3RenderCommandEncoder::GenerateMipmaps(ref_ptr<Image> image) {
  // TODO(benvanik): this.
  LOG(WARNING) << "GenerateMipmaps not yet implemented";
}

ES3RenderPassCommandEncoder::ES3RenderPassCommandEncoder(
    CommandBuffer* command_buffer)
    : RenderPassCommandEncoder(command_buffer),
      common_encoder_(command_buffer) {}

ES3RenderPassCommandEncoder::~ES3RenderPassCommandEncoder() = default;

void ES3RenderPassCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  common_encoder_.PipelineBarrier(source_stage_mask, target_stage_mask,
                                  dependency_flags);
}

void ES3RenderPassCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  common_encoder_.MemoryBarrier(source_stage_mask, target_stage_mask,
                                dependency_flags, source_access_mask,
                                target_access_mask);
}

void ES3RenderPassCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  common_encoder_.BufferBarrier(source_stage_mask, target_stage_mask,
                                dependency_flags, source_access_mask,
                                target_access_mask, std::move(buffer), offset,
                                length);
}

void ES3RenderPassCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  common_encoder_.ImageBarrier(source_stage_mask, target_stage_mask,
                               dependency_flags, source_access_mask,
                               target_access_mask, source_layout, target_layout,
                               std::move(image), layer_range);
}

void ES3RenderPassCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  common_encoder_.WaitFences(fences);
}

void ES3RenderPassCommandEncoder::ClearColorAttachment(
    int color_attachment_index, ClearColor clear_color,
    ArrayView<ClearRect> clear_rects) {
  for (const auto& clear_rect : clear_rects) {
    glScissor(clear_rect.rect.origin.x, clear_rect.rect.origin.y,
              clear_rect.rect.size.width, clear_rect.rect.size.height);
    glClearBufferuiv(GL_COLOR, color_attachment_index, clear_color.uint_value);
  }
  glScissor(scissor_rect_.origin.x, scissor_rect_.origin.y,
            scissor_rect_.size.width, scissor_rect_.size.height);
}

void ES3RenderPassCommandEncoder::ClearDepthStencilAttachment(
    float depth_value, uint32_t stencil_value,
    ArrayView<ClearRect> clear_rects) {
  for (const auto& clear_rect : clear_rects) {
    glScissor(clear_rect.rect.origin.x, clear_rect.rect.origin.y,
              clear_rect.rect.size.width, clear_rect.rect.size.height);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, depth_value, stencil_value);
  }
  glScissor(scissor_rect_.origin.x, scissor_rect_.origin.y,
            scissor_rect_.size.width, scissor_rect_.size.height);
}

void ES3RenderPassCommandEncoder::BeginRenderPass(
    ref_ptr<RenderPass> render_pass, ref_ptr<Framebuffer> framebuffer,
    ArrayView<ClearColor> clear_colors) {
  render_pass_ = std::move(render_pass);
  framebuffer_ = std::move(framebuffer);
  clear_colors_ = std::vector<ClearColor>(clear_colors);
  subpass_index_ = 0;

  DCHECK_LE(framebuffer_->attachments().size(), 64);
  used_attachments_ = 0;

  // TODO(benvanik): cache many VAOs to use (bitmaks of enabled attribs?)
  glGenVertexArrays(1, &scratch_vao_id_);
  DCHECK_NE(scratch_vao_id_, 0);
  glBindVertexArray(scratch_vao_id_);

  glGenFramebuffers(1, &scratch_framebuffer_id_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, scratch_framebuffer_id_);

  PrepareSubpass();
}

void ES3RenderPassCommandEncoder::NextSubpass() {
  DCHECK_LT(subpass_index_ + 1, render_pass_->subpasses().size());
  ++subpass_index_;
  PrepareSubpass();
}

void ES3RenderPassCommandEncoder::PrepareSubpass() {
  const auto& subpass = render_pass_->subpasses()[subpass_index_];

  // TODO(benvanik): cache the framebuffer objects per subpass?

  // Setup color attachments on the framebuffer.
  GLenum draw_buffers[8] = {GL_NONE};
  DCHECK_LE(subpass.color_attachments.size(), count_of(draw_buffers));
  for (int i = 0; i < subpass.color_attachments.size(); ++i) {
    const auto& attachment_ref = subpass.color_attachments[i];
    if (attachment_ref.index == RenderPass::AttachmentReference::kUnused) {
      glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                             GL_TEXTURE_2D, 0, 0);
      continue;
    }
    auto image = framebuffer_->attachments()[attachment_ref.index]
                     ->image()
                     .As<ES3Image>();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, image->texture_id(), 0);
    draw_buffers[i] = GL_COLOR_ATTACHMENT0 + i;
  }
  glDrawBuffers(static_cast<GLsizei>(subpass.color_attachments.size()),
                draw_buffers);

  // Setup depth/stencil (if present).
  // TODO(benvanik): depth_stencil_attachment

  // TODO(benvanik): input_attachments
  // TODO(benvanik): resolve_attachments

  // TODO(benvanik): glCheckFramebufferStatus for safety.

  // Prepare the framebuffer for use.
  for (int i = 0; i < subpass.color_attachments.size(); ++i) {
    const auto& attachment_ref = subpass.color_attachments[i];
    if (attachment_ref.index == RenderPass::AttachmentReference::kUnused) {
      continue;
    }
    if ((used_attachments_ & (1ull << attachment_ref.index)) == 0) {
      // This attachment has not yet been used in this render pass; clear it if
      // needed.
      used_attachments_ |= (1ull << attachment_ref.index);
      const auto& attachment =
          render_pass_->attachments()[attachment_ref.index];
      if (attachment.load_op == RenderPass::LoadOp::kClear &&
          i < clear_colors_.size()) {
        ClearColorAttachment(i, clear_colors_[i],
                             {ClearRect(0, 0, 16 * 1024, 16 * 1024)});
      }
    }
  }
}

void ES3RenderPassCommandEncoder::EndRenderPass() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &scratch_framebuffer_id_);
  scratch_framebuffer_id_ = 0;

  glBindVertexArray(0);
  glDeleteVertexArrays(1, &scratch_vao_id_);
  scratch_vao_id_ = 0;
}

void ES3RenderPassCommandEncoder::SetScissors(int first_scissor,
                                              ArrayView<Rect2D> scissors) {
  // TODO(benvanik): multiple scissors?
  DCHECK_EQ(first_scissor, 0);
  DCHECK_EQ(scissors.size(), 1);
  const Rect2D& scissor = scissors[0];
  scissor_rect_ = scissor;
  glScissor(scissor.origin.x, scissor.origin.y, scissor.size.width,
            scissor.size.height);
}

void ES3RenderPassCommandEncoder::SetViewports(int first_viewport,
                                               ArrayView<Viewport> viewports) {
  // TODO(benvanik): multiple viewports?
  DCHECK_EQ(first_viewport, 0);
  DCHECK_EQ(viewports.size(), 1);
  const Viewport& viewport = viewports[0];
  glViewport(static_cast<GLint>(viewport.x), static_cast<GLint>(viewport.y),
             static_cast<GLsizei>(viewport.width),
             static_cast<GLsizei>(viewport.height));
  glDepthRangef(viewport.min_depth, viewport.max_depth);
}

void ES3RenderPassCommandEncoder::SetLineWidth(float line_width) {
  glLineWidth(line_width);
}

void ES3RenderPassCommandEncoder::SetDepthBias(float depth_bias_constant_factor,
                                               float depth_bias_clamp,
                                               float depth_bias_slope_factor) {
  // TODO(benvanik): this.
  LOG(WARNING) << "SetDepthBias not yet implemented";
}

void ES3RenderPassCommandEncoder::SetDepthBounds(float min_depth_bounds,
                                                 float max_depth_bounds) {
  // TODO(benvanik): this.
  LOG(WARNING) << "SetDepthBounds not yet implemented";
}

void ES3RenderPassCommandEncoder::SetStencilCompareMask(
    StencilFaceFlag face_mask, uint32_t compare_mask) {
  // TODO(benvanik): this.
  LOG(WARNING) << "SetStencilCompareMask not yet implemented";
}

void ES3RenderPassCommandEncoder::SetStencilWriteMask(StencilFaceFlag face_mask,
                                                      uint32_t write_mask) {
  GLenum face = GetFaceFromFaceMask(face_mask);
  glStencilMaskSeparate(face, write_mask);
}

void ES3RenderPassCommandEncoder::SetStencilReference(StencilFaceFlag face_mask,
                                                      uint32_t reference) {
  // TODO(benvanik): this.
  LOG(WARNING) << "SetStencilReference not yet implemented";
}

void ES3RenderPassCommandEncoder::SetBlendConstants(
    const float blend_constants[4]) {
  glBlendColor(blend_constants[0], blend_constants[1], blend_constants[2],
               blend_constants[3]);
}

void ES3RenderPassCommandEncoder::BindPipeline(
    ref_ptr<RenderPipeline> pipeline) {
  if (pipeline == pipeline_) {
    // TODO(benvanik): try harder to dedupe.
    return;
  }

  // Set active shader program.
  auto program = pipeline.As<ES3RenderPipeline>()->program();
  glUseProgram(program->program_id());

  // Set render state and cache values we'll use frequently.
  const auto& render_state = pipeline->render_state();

  // Setup our vertex input mirror state. This may reuse previous buffer
  // bindings (if any) and get updated by future BindVertexBuffers calls.
  const auto& vertex_bindings = render_state.vertex_input_state.vertex_bindings;
  for (const auto& vertex_binding : vertex_bindings) {
    auto& binding_slot = vertex_buffer_bindings_[vertex_binding.binding];
    binding_slot.stride = vertex_binding.stride;
    binding_slot.input_rate = vertex_binding.input_rate;
  }
  const auto& vertex_attributes =
      render_state.vertex_input_state.vertex_attributes;
  vertex_buffer_attribs_.resize(kMaxVertexInputs);
  for (auto& attrib_slot : vertex_buffer_attribs_) {
    attrib_slot.binding = -1;
  }
  for (const auto& vertex_attribute : vertex_attributes) {
    auto& attrib_slot = vertex_buffer_attribs_[vertex_attribute.location];
    attrib_slot.binding = vertex_attribute.binding;
    attrib_slot.offset = vertex_attribute.offset;
    attrib_slot.format = vertex_attribute.format;
    const auto& binding_slot = vertex_buffer_bindings_[attrib_slot.binding];
    glVertexAttribDivisor(
        vertex_attribute.location,
        binding_slot.input_rate == VertexInputRate::kVertex ? 0 : 1);
  }
  for (int i = 0; i < vertex_buffer_attribs_.size(); ++i) {
    const auto& attrib_slot = vertex_buffer_attribs_[i];
    if (attrib_slot.binding == -1) {
      if (vertex_attrib_enable_mask_ & (1 << i)) {
        vertex_attrib_enable_mask_ ^= (1 << i);
        glDisableVertexAttribArray(i);
      }
    } else {
      if ((vertex_attrib_enable_mask_ & (1 << i)) == 0) {
        vertex_attrib_enable_mask_ |= (1 << i);
        glEnableVertexAttribArray(i);
      }
    }
  }
  vertex_inputs_dirty_ = true;

  // TODO(benvanik): input_assembly_state.is_primitive_restart_enabled()
  static const GLenum kPrimitiveTopologyLookup[] = {
      GL_POINTS,                    // kPointList
      GL_LINES,                     // kLineList
      GL_LINE_STRIP,                // kLineStrip
      GL_TRIANGLES,                 // kTriangleList
      GL_TRIANGLE_STRIP,            // kTriangleStrip
      GL_TRIANGLE_FAN,              // kTriangleFan
      GL_LINES_ADJACENCY,           // kLineListWithAdjacency
      GL_LINE_STRIP_ADJACENCY,      // kLineStripWithAdjacency
      GL_TRIANGLES_ADJACENCY,       // kTriangleListWithAdjacency
      GL_TRIANGLE_STRIP_ADJACENCY,  // kTriangleStripWithAdjacency
      GL_PATCHES,                   // kPatchList
  };
  int primitive_topology_index =
      static_cast<int>(render_state.input_assembly_state.primitive_topology());
  DCHECK_LE(primitive_topology_index, count_of(kPrimitiveTopologyLookup));
  draw_primitive_mode_ = kPrimitiveTopologyLookup[primitive_topology_index];

  // TODO(benvanik): tessellation_state.patch_control_points

  // TODO(benvanik): viewport_state.count

  // TODO(benvanik): rasterization_state

  // TODO(benvanik): multisample_state

  // TODO(benvanik): depth_stencil_state

  // TODO(benvanik): color_blend_state
}

void ES3RenderPassCommandEncoder::BindResourceSet(
    ref_ptr<ResourceSet> resource_set, ArrayView<size_t> dynamic_offsets) {
  resource_set_ = resource_set;
  // TODO(benvanik): reserve to avoid allocations.
  dynamic_offsets_ = dynamic_offsets;

  // Ensure that UpdateResourceSet is called before we draw again.
  resource_set_dirty_ = true;
}

void ES3RenderPassCommandEncoder::PushConstants(
    ref_ptr<PipelineLayout> pipeline_layout, ShaderStageFlag stage_mask,
    size_t offset, const void* data, size_t data_length) {
  // TODO(benvanik): this.
  LOG(WARNING) << "PushConstants not yet implemented";
}

void ES3RenderPassCommandEncoder::BindVertexBuffers(
    int first_binding, ArrayView<ref_ptr<Buffer>> buffers) {
  BindVertexBuffers(first_binding, buffers, {});
}

void ES3RenderPassCommandEncoder::BindVertexBuffers(
    int first_binding, ArrayView<ref_ptr<Buffer>> buffers,
    ArrayView<size_t> buffer_offsets) {
  for (int i = 0; i < buffers.size(); ++i) {
    int binding_index = first_binding + i;
    auto& binding_slot = vertex_buffer_bindings_[binding_index];
    binding_slot.buffer = buffers[i];
    binding_slot.buffer_offset = buffer_offsets.empty() ? 0 : buffer_offsets[i];
  }

  // Ensure that UpdateVertexInputs is called before we draw again.
  vertex_inputs_dirty_ = true;
}

void ES3RenderPassCommandEncoder::BindIndexBuffer(ref_ptr<Buffer> buffer,
                                                  size_t buffer_offset,
                                                  IndexElementType index_type) {
  index_buffer_ = buffer;
  index_buffer_offset_ = buffer_offset;
  switch (index_type) {
    case IndexElementType::kUint16:
      index_buffer_type_ = GL_UNSIGNED_SHORT;
      break;
    case IndexElementType::kUint32:
      index_buffer_type_ = GL_UNSIGNED_INT;
      break;
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.As<ES3Buffer>()->buffer_id());
}

void ES3RenderPassCommandEncoder::UpdateResourceSet() {
  if (!resource_set_dirty_) {
    // No resource set changes.
    return;
  }
  resource_set_dirty_ = false;

  uint32_t new_texture_binding_mask = 0;
  uint32_t new_uniform_buffer_binding_mask = 0;
  if (resource_set_) {
    const auto& binding_slots = resource_set_->layout()->binding_slots();
    for (int i = 0; i < binding_slots.size(); ++i) {
      const PipelineLayout::BindingSlot& binding_slot = binding_slots[i];
      const ResourceSet::BindingValue& binding_value =
          resource_set_->binding_values()[i];

      // TODO(benvanik): support binding arrays.
      DCHECK_EQ(binding_slot.array_count, 1);

      switch (binding_slot.type) {
        case PipelineLayout::BindingSlot::Type::kCombinedImageSampler: {
          // TODO(benvanik): validate during ResourceSet init.
          DCHECK(binding_value.image_view);
          DCHECK(binding_value.sampler);
          auto image = binding_value.image_view->image().As<ES3Image>();
          auto sampler = binding_value.sampler.As<ES3Sampler>();
          glActiveTexture(GL_TEXTURE0 + binding_slot.binding);
          glBindTexture(image->target(), image->texture_id());
          glBindSampler(binding_slot.binding, sampler->sampler_id());
          new_texture_binding_mask |= 1 << binding_slot.binding;
          break;
        }
        case PipelineLayout::BindingSlot::Type::kUniformBuffer: {
          // TODO(benvanik): validate during ResourceSet init.
          DCHECK(binding_value.buffer);
          auto buffer = binding_value.buffer.As<ES3Buffer>();
          glBindBuffer(GL_UNIFORM_BUFFER, buffer->buffer_id());
          size_t bind_offset = binding_value.buffer_offset;
          size_t bind_length = binding_value.buffer_length != -1
                                   ? binding_value.buffer_length
                                   : buffer->allocation_size();
          glBindBufferRange(GL_UNIFORM_BUFFER, binding_slot.binding,
                            buffer->buffer_id(), bind_offset, bind_length);
          new_uniform_buffer_binding_mask |= 1 << binding_slot.binding;
          break;
        }
        default:
          // Not yet implemented.
          DCHECK(false);
          break;
      }
    }
  }

  // Unbind any unused slots. We may not need to do this but it ensures clean
  // state when debugging.
  if (new_texture_binding_mask != texture_binding_mask_) {
    uint32_t unused_mask = (new_texture_binding_mask ^ texture_binding_mask_) &
                           texture_binding_mask_;
    if (unused_mask) {
      const int kMaxTextureUnit = 32;
      for (int i = 0; i < kMaxTextureUnit; ++i) {
        if (unused_mask & (1 << i)) {
          // This slot is now unused.
          glActiveTexture(GL_TEXTURE0 + i);
          glBindTexture(GL_TEXTURE_2D, 0);
          glBindSampler(i, 0);
        }
      }
    }
    texture_binding_mask_ = new_texture_binding_mask;
  }
  if (new_uniform_buffer_binding_mask != uniform_buffer_binding_mask_) {
    uint32_t unused_mask =
        (new_uniform_buffer_binding_mask ^ uniform_buffer_binding_mask_) &
        uniform_buffer_binding_mask_;
    if (unused_mask) {
      const int kMaxBindingUnit = 32;
      for (int i = 0; i < kMaxBindingUnit; ++i) {
        if (unused_mask & (1 << i)) {
          // This slot is now unused.
          glBindBufferBase(GL_UNIFORM_BUFFER, i, 0);
        }
      }
    }
    uniform_buffer_binding_mask_ = new_uniform_buffer_binding_mask;
  }
}

void ES3RenderPassCommandEncoder::UpdateVertexInputs() {
  if (!vertex_inputs_dirty_) {
    // No vertex input bindings or configurations have changed, so ignore.
    return;
  }
  vertex_inputs_dirty_ = false;

  ref_ptr<Buffer> bound_buffer;
  for (int i = 0; i < vertex_buffer_attribs_.size(); ++i) {
    // Set array buffer binding to the buffer this attribute uses.
    const auto& attrib_slot = vertex_buffer_attribs_[i];
    if (attrib_slot.binding == -1) {
      continue;
    }
    const auto& binding_slot = vertex_buffer_bindings_[attrib_slot.binding];
    if (bound_buffer != binding_slot.buffer) {
      bound_buffer = binding_slot.buffer;
      glBindBuffer(GL_ARRAY_BUFFER,
                   binding_slot.buffer.As<ES3Buffer>()->buffer_id());
    }

    // Setup attribute based on VertexFormat.
    VertexFormat format = attrib_slot.format;
    GLint size = format.component_count();
    GLenum type = GL_NONE;
    bool integer_format = format.component_format() == ComponentFormat::kSInt ||
                          format.component_format() == ComponentFormat::kUInt;
    bool normalized = format.component_format() == ComponentFormat::kSNorm ||
                      format.component_format() == ComponentFormat::kUNorm;
    if (format == VertexFormats::kW2X10Y10Z10UNorm) {
      type = GL_UNSIGNED_INT_2_10_10_10_REV;
    } else if (format == VertexFormats::kW2X10Y10Z10SNorm) {
      type = GL_INT_2_10_10_10_REV;
    } else {
      switch (format.component_format()) {
        default:
        case ComponentFormat::kSFloat:
          switch (format.component_bits_x()) {
            case 16:
              type = GL_HALF_FLOAT;
              break;
            case 32:
              type = GL_FLOAT;
              break;
          }
          break;
        case ComponentFormat::kSNorm:
        case ComponentFormat::kSInt:
          switch (format.component_bits_x()) {
            case 8:
              type = GL_BYTE;
              break;
            case 16:
              type = GL_SHORT;
              break;
            case 32:
              type = GL_INT;
              break;
          }
          break;
        case ComponentFormat::kUNorm:
        case ComponentFormat::kUInt:
          switch (format.component_bits_x()) {
            case 8:
              type = GL_UNSIGNED_BYTE;
              break;
            case 16:
              type = GL_UNSIGNED_SHORT;
              break;
            case 32:
              type = GL_UNSIGNED_INT;
              break;
          }
          break;
      }
    }
    DCHECK_NE(type, GL_NONE);
    if (integer_format) {
      glVertexAttribIPointer(
          i, size, type, static_cast<GLsizei>(binding_slot.stride),
          reinterpret_cast<void*>(binding_slot.buffer_offset +
                                  attrib_slot.offset));
    } else {
      glVertexAttribPointer(i, size, type, normalized ? GL_TRUE : GL_FALSE,
                            static_cast<GLsizei>(binding_slot.stride),
                            reinterpret_cast<void*>(binding_slot.buffer_offset +
                                                    attrib_slot.offset));
    }
  }
}

void ES3RenderPassCommandEncoder::Draw(int vertex_count, int instance_count,
                                       int first_vertex, int first_instance) {
  // TODO(benvanik): modify gl_InstanceID? use CPU glDrawArraysIndirect?
  DCHECK_EQ(first_instance, 0);

  UpdateResourceSet();
  UpdateVertexInputs();

  // EWW: nvidia drivers on linux leak a few statics. It'd be nice to find a
  //      better place for this or find out why it leaks.
  debugging::LeakCheckDisabler leak_check_disabler;

  if (instance_count > 1) {
    glDrawArraysInstanced(draw_primitive_mode_, first_vertex, vertex_count,
                          instance_count);
  } else {
    glDrawArrays(draw_primitive_mode_, first_vertex, vertex_count);
  }
}

void ES3RenderPassCommandEncoder::DrawIndexed(int index_count,
                                              int instance_count,
                                              int first_index,
                                              int vertex_offset,
                                              int first_instance) {
  // TODO(benvanik): modify gl_InstanceID? use CPU glDrawArraysIndirect?
  DCHECK_EQ(first_index, 0);
  DCHECK_EQ(vertex_offset, 0);
  DCHECK_EQ(first_instance, 0);

  UpdateResourceSet();
  UpdateVertexInputs();

  DCHECK(index_buffer_);
  size_t type_size = (index_buffer_type_ == GL_UNSIGNED_INT) ? 4 : 2;
  if (instance_count > 1) {
    glDrawElementsInstanced(draw_primitive_mode_, index_count,
                            index_buffer_type_,
                            reinterpret_cast<const void*>(
                                index_buffer_offset_ + first_index * type_size),
                            instance_count);
  } else {
    glDrawElements(draw_primitive_mode_, index_count, index_buffer_type_,
                   reinterpret_cast<const void*>(index_buffer_offset_ +
                                                 first_index * type_size));
  }
}

void ES3RenderPassCommandEncoder::DrawIndirect(ref_ptr<Buffer> buffer,
                                               size_t buffer_offset,
                                               int draw_count, size_t stride) {
  UpdateResourceSet();
  UpdateVertexInputs();

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer.As<ES3Buffer>()->buffer_id());
  for (int i = 0; i < draw_count; ++i) {
    glDrawArraysIndirect(draw_primitive_mode_, reinterpret_cast<const void*>(
                                                   buffer_offset + i * stride));
  }
}

void ES3RenderPassCommandEncoder::DrawIndexedIndirect(ref_ptr<Buffer> buffer,
                                                      size_t buffer_offset,
                                                      int draw_count,
                                                      size_t stride) {
  UpdateResourceSet();
  UpdateVertexInputs();

  DCHECK(index_buffer_);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer.As<ES3Buffer>()->buffer_id());
  for (int i = 0; i < draw_count; ++i) {
    glDrawElementsIndirect(
        draw_primitive_mode_, index_buffer_type_,
        reinterpret_cast<const void*>(buffer_offset + i * stride));
  }
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
