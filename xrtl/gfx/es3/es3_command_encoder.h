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

#ifndef XRTL_GFX_ES3_ES3_COMMAND_ENCODER_H_
#define XRTL_GFX_ES3_ES3_COMMAND_ENCODER_H_

#include <utility>
#include <vector>

#include "xrtl/gfx/command_encoder.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/framebuffer.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// Concrete command encoder implementations for GL.
// These are used to execute commands against the current GL context and are
// called by the MemoryCommandBuffer implementation decoding a previously
// generated command buffer.

class ES3TransferCommandEncoder : public TransferCommandEncoder {
 public:
  explicit ES3TransferCommandEncoder(CommandBuffer* command_buffer);
  ~ES3TransferCommandEncoder();

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override;

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override;

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override;

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override;

  void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                  uint8_t value) override;

  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const void* source_data,
                    size_t source_data_length) override;

  void CopyBuffer(ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
                  ArrayView<CopyBufferRegion> regions) override;

  void CopyImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 ArrayView<CopyImageRegion> regions) override;

  void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyBufferImageRegion> regions) override;

  void CopyImageToBuffer(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Buffer> target_buffer,
                         ArrayView<CopyBufferImageRegion> regions) override;

 protected:
  friend class ES3ComputeCommandEncoder;
  friend class ES3RenderCommandEncoder;
  friend class ES3RenderPassCommandEncoder;

  // These methods are not transfer related but here because we reuse this
  // type as the common encoder.
  void SetFence(ref_ptr<CommandFence> fence,
                PipelineStageFlag pipeline_stage_mask);
  void ResetFence(ref_ptr<CommandFence> fence,
                  PipelineStageFlag pipeline_stage_mask);
  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences);
  void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                       ClearColor clear_color,
                       ArrayView<Image::LayerRange> ranges);
};

class ES3ComputeCommandEncoder : public ComputeCommandEncoder {
 public:
  explicit ES3ComputeCommandEncoder(CommandBuffer* command_buffer);
  ~ES3ComputeCommandEncoder();

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override;

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override;

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override;

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override;

  void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                  uint8_t value) override;

  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const void* source_data,
                    size_t source_data_length) override;

  void CopyBuffer(ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
                  ArrayView<CopyBufferRegion> regions) override;

  void CopyImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 ArrayView<CopyImageRegion> regions) override;

  void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyBufferImageRegion> regions) override;

  void CopyImageToBuffer(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Buffer> target_buffer,
                         ArrayView<CopyBufferImageRegion> regions) override;

  void SetFence(ref_ptr<CommandFence> fence,
                PipelineStageFlag pipeline_stage_mask) override;

  void ResetFence(ref_ptr<CommandFence> fence,
                  PipelineStageFlag pipeline_stage_mask) override;

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override;

  void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                       ClearColor clear_color,
                       ArrayView<Image::LayerRange> ranges) override;

  void BindPipeline(ref_ptr<ComputePipeline> pipeline) override;

  void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set,
                       ArrayView<size_t> dynamic_offsets) override;

  void PushConstants(ref_ptr<PipelineLayout> pipeline_layout,
                     ShaderStageFlag stage_mask, size_t offset,
                     const void* data, size_t data_length) override;

  void Dispatch(int group_count_x, int group_count_y,
                int group_count_z) override;

  void DispatchIndirect(ref_ptr<Buffer> buffer, size_t offset) override;

 private:
  ES3TransferCommandEncoder common_encoder_;
};

class ES3RenderCommandEncoder : public RenderCommandEncoder {
 public:
  explicit ES3RenderCommandEncoder(CommandBuffer* command_buffer);
  ~ES3RenderCommandEncoder();

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override;

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override;

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override;

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override;

  void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                  uint8_t value) override;

  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const void* source_data,
                    size_t source_data_length) override;

  void CopyBuffer(ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
                  ArrayView<CopyBufferRegion> regions) override;

  void CopyImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 ArrayView<CopyImageRegion> regions) override;

  void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyBufferImageRegion> regions) override;

  void CopyImageToBuffer(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Buffer> target_buffer,
                         ArrayView<CopyBufferImageRegion> regions) override;

  void SetFence(ref_ptr<CommandFence> fence,
                PipelineStageFlag pipeline_stage_mask) override;

  void ResetFence(ref_ptr<CommandFence> fence,
                  PipelineStageFlag pipeline_stage_mask) override;

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override;

  void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                       ClearColor clear_color,
                       ArrayView<Image::LayerRange> ranges) override;

  void ClearDepthStencilImage(ref_ptr<Image> image, Image::Layout image_layout,
                              float depth_value, uint32_t stencil_value,
                              ArrayView<Image::LayerRange> ranges) override;

  void BlitImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 Sampler::Filter scaling_filter,
                 ArrayView<BlitImageRegion> regions) override;

  void ResolveImage(ref_ptr<Image> source_image,
                    Image::Layout source_image_layout,
                    ref_ptr<Image> target_image,
                    Image::Layout target_image_layout,
                    ArrayView<CopyImageRegion> regions) override;

  void GenerateMipmaps(ref_ptr<Image> image) override;

 private:
  ES3TransferCommandEncoder common_encoder_;
};

class ES3RenderPassCommandEncoder : public RenderPassCommandEncoder {
 public:
  explicit ES3RenderPassCommandEncoder(CommandBuffer* command_buffer);
  ~ES3RenderPassCommandEncoder();

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override;

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override;

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override;

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override;

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override;

  void ClearColorAttachment(int color_attachment_index, ClearColor clear_color,
                            ArrayView<ClearRect> clear_rects) override;

  void ClearDepthStencilAttachment(float depth_value, uint32_t stencil_value,
                                   ArrayView<ClearRect> clear_rects) override;

  void BeginRenderPass(ref_ptr<RenderPass> render_pass,
                       ref_ptr<Framebuffer> framebuffer,
                       ArrayView<ClearColor> clear_colors);
  void NextSubpass() override;
  void EndRenderPass();

  void SetScissors(int first_scissor, ArrayView<Rect2D> scissors) override;

  void SetViewports(int first_viewport, ArrayView<Viewport> viewports) override;

  void SetLineWidth(float line_width) override;

  void SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp,
                    float depth_bias_slope_factor) override;

  void SetDepthBounds(float min_depth_bounds, float max_depth_bounds) override;

  void SetStencilCompareMask(StencilFaceFlag face_mask,
                             uint32_t compare_mask) override;

  void SetStencilWriteMask(StencilFaceFlag face_mask,
                           uint32_t write_mask) override;

  void SetStencilReference(StencilFaceFlag face_mask,
                           uint32_t reference) override;

  void SetBlendConstants(const float blend_constants[4]) override;

  void BindPipeline(ref_ptr<RenderPipeline> pipeline) override;

  void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set,
                       ArrayView<size_t> dynamic_offsets) override;

  void PushConstants(ref_ptr<PipelineLayout> pipeline_layout,
                     ShaderStageFlag stage_mask, size_t offset,
                     const void* data, size_t data_length) override;

  void BindVertexBuffers(int first_binding,
                         ArrayView<ref_ptr<Buffer>> buffers) override;
  void BindVertexBuffers(int first_binding, ArrayView<ref_ptr<Buffer>> buffers,
                         ArrayView<size_t> buffer_offsets) override;

  void BindIndexBuffer(ref_ptr<Buffer> buffer, size_t buffer_offset,
                       IndexElementType index_type) override;

  void Draw(int vertex_count, int instance_count, int first_vertex,
            int first_instance) override;

  void DrawIndexed(int index_count, int instance_count, int first_index,
                   int vertex_offset, int first_instance) override;

  void DrawIndirect(ref_ptr<Buffer> buffer, size_t buffer_offset,
                    int draw_count, size_t stride) override;

  void DrawIndexedIndirect(ref_ptr<Buffer> buffer, size_t buffer_offset,
                           int draw_count, size_t stride) override;

 private:
  // NOTE: all state is retained for the lifetime of the parent command buffer.

  // Prepares the current subpass state.
  void PrepareSubpass();
  // Finishes a subpass before the next executes (or the render pass ends).
  void FinishSubpass();

  // Refreshes the GL state machine with the given XRTL state.
  void RefreshVertexInputState(
      const RenderState::VertexInputState& vertex_input_state);
  void RefreshInputAssemblyState(
      const RenderState::InputAssemblyState& input_assembly_state);
  void RefreshTessellationState(
      const RenderState::TessellationState& tessellation_state);
  void RefreshViewportState(const RenderState::ViewportState& viewport_state);
  void RefreshRasterizationState(
      const RenderState::RasterizationState& rasterization_state);
  void RefreshMultisampleState(
      const RenderState::MultisampleState& multisample_state);
  void RefreshDepthStencilState(
      const RenderState::DepthStencilState& depth_stencil_state);
  void RefreshColorBlendState(
      int attachment_index,
      const RenderState::ColorBlendAttachmentState& attachment_state);

  // Updates push constant data for the current pipeline.
  void UpdatePushConstants();
  // Updates the resource set binding based on the current pipeline and set.
  void UpdateResourceSets();
  // Updates all vertex inputs based on the current bindings and pipeline.
  void UpdateVertexInputs();

  ES3TransferCommandEncoder common_encoder_;

  ref_ptr<RenderPass> render_pass_;
  ref_ptr<Framebuffer> framebuffer_;
  std::vector<ClearColor> clear_colors_;
  int subpass_index_ = 0;
  uint64_t used_attachments_ = 0;
  GLuint scratch_framebuffer_id_ = 0;

  ref_ptr<RenderPipeline> pipeline_;
  GLenum draw_primitive_mode_ = GL_TRIANGLES;

  Rect2D scissor_rect_{0, 0, 16 * 1024, 16 * 1024};

  ref_ptr<ResourceSet> resource_sets_[kMaxResourceSetCount];
  std::vector<size_t> dynamic_offsets_[kMaxResourceSetCount];
  bool resource_sets_dirty_ = true;
  uint32_t texture_binding_mask_ = 0;
  uint32_t uniform_buffer_binding_mask_ = 0;
  std::vector<uint8_t> push_constant_data_;
  bool push_constants_dirty_ = true;

  // Array indices are binding and location, respectively.
  struct VertexBufferBinding {
    ref_ptr<Buffer> buffer;
    size_t buffer_offset = 0;
    size_t stride = 0;
    VertexInputRate input_rate = VertexInputRate::kVertex;
  };
  VertexBufferBinding vertex_buffer_bindings_[kMaxVertexInputs];
  struct VertexBufferAttribs {
    int binding = -1;  // -1 indicates unused
    size_t offset = 0;
    VertexFormat format = VertexFormats::kUndefined;
  };
  FixedVector<VertexBufferAttribs, kMaxVertexInputs> vertex_buffer_attribs_;
  bool vertex_inputs_dirty_ = true;
  uint32_t vertex_attrib_enable_mask_ = 0;
  GLuint scratch_vao_id_ = 0;

  ref_ptr<Buffer> index_buffer_;
  size_t index_buffer_offset_ = 0;
  GLenum index_buffer_type_ = GL_UNSIGNED_INT;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_COMMAND_ENCODER_H_
