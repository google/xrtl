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

#ifndef XRTL_GFX_TESTING_PARTIAL_COMMAND_ENCODER_H_
#define XRTL_GFX_TESTING_PARTIAL_COMMAND_ENCODER_H_

#include <utility>

#include "xrtl/gfx/command_encoder.h"

namespace xrtl {
namespace gfx {
namespace testing {

// Partial encoders that have default no-op implementations of all the encoder
// interface methods. This enables tests to write encoders that only override
// the methods the tests are interested in instead of needing all methods
// stubbed out.

class PartialTransferCommandEncoder : public TransferCommandEncoder {
 public:
  explicit PartialTransferCommandEncoder(CommandBuffer* command_buffer)
      : TransferCommandEncoder(command_buffer) {}

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override {}

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override {}

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override {}

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override {}

  void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                  uint8_t value) override {}

  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const void* source_data,
                    size_t source_data_length) override {}

  void CopyBuffer(ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
                  ArrayView<CopyBufferRegion> regions) override {}

  void CopyImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 ArrayView<CopyImageRegion> regions) override {}

  void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyBufferImageRegion> regions) override {}

  void CopyImageToBuffer(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Buffer> target_buffer,
                         ArrayView<CopyBufferImageRegion> regions) override {}
};

class PartialComputeCommandEncoder : public ComputeCommandEncoder {
 public:
  explicit PartialComputeCommandEncoder(CommandBuffer* command_buffer)
      : ComputeCommandEncoder(command_buffer) {}

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override {}

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override {}

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override {}

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override {}

  void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                  uint8_t value) override {}

  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const void* source_data,
                    size_t source_data_length) override {}

  void CopyBuffer(ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
                  ArrayView<CopyBufferRegion> regions) override {}

  void CopyImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 ArrayView<CopyImageRegion> regions) override {}

  void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyBufferImageRegion> regions) override {}

  void CopyImageToBuffer(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Buffer> target_buffer,
                         ArrayView<CopyBufferImageRegion> regions) override {}

  void SetFence(ref_ptr<CommandFence> fence,
                PipelineStageFlag pipeline_stage_mask) override {}

  void ResetFence(ref_ptr<CommandFence> fence,
                  PipelineStageFlag pipeline_stage_mask) override {}

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override {}

  void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                       ClearColor clear_color,
                       ArrayView<Image::LayerRange> ranges) override {}

  void BindPipeline(ref_ptr<ComputePipeline> pipeline) override {}

  void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set,
                       ArrayView<size_t> dynamic_offsets) override {}

  void PushConstants(ref_ptr<PipelineLayout> pipeline_layout,
                     ShaderStageFlag stage_mask, size_t offset,
                     const void* data, size_t data_length) override {}

  void Dispatch(int group_count_x, int group_count_y,
                int group_count_z) override {}

  void DispatchIndirect(ref_ptr<Buffer> buffer, size_t offset) override {}
};

class PartialRenderCommandEncoder : public RenderCommandEncoder {
 public:
  explicit PartialRenderCommandEncoder(CommandBuffer* command_buffer)
      : RenderCommandEncoder(command_buffer) {}

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override {}

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override {}

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override {}

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override {}

  void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                  uint8_t value) override {}

  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const void* source_data,
                    size_t source_data_length) override {}

  void CopyBuffer(ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
                  ArrayView<CopyBufferRegion> regions) override {}

  void CopyImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 ArrayView<CopyImageRegion> regions) override {}

  void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyBufferImageRegion> regions) override {}

  void CopyImageToBuffer(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Buffer> target_buffer,
                         ArrayView<CopyBufferImageRegion> regions) override {}

  void SetFence(ref_ptr<CommandFence> fence,
                PipelineStageFlag pipeline_stage_mask) override {}

  void ResetFence(ref_ptr<CommandFence> fence,
                  PipelineStageFlag pipeline_stage_mask) override {}

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override {}

  void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                       ClearColor clear_color,
                       ArrayView<Image::LayerRange> ranges) override {}

  void ClearDepthStencilImage(ref_ptr<Image> image, Image::Layout image_layout,
                              float depth_value, uint32_t stencil_value,
                              ArrayView<Image::LayerRange> ranges) override {}

  void BlitImage(ref_ptr<Image> source_image, Image::Layout source_image_layout,
                 ref_ptr<Image> target_image, Image::Layout target_image_layout,
                 Sampler::Filter scaling_filter,
                 ArrayView<BlitImageRegion> regions) override {}

  void ResolveImage(ref_ptr<Image> source_image,
                    Image::Layout source_image_layout,
                    ref_ptr<Image> target_image,
                    Image::Layout target_image_layout,
                    ArrayView<CopyImageRegion> regions) override {}

  void GenerateMipmaps(ref_ptr<Image> image) override {}
};

class PartialRenderPassCommandEncoder : public RenderPassCommandEncoder {
 public:
  explicit PartialRenderPassCommandEncoder(CommandBuffer* command_buffer)
      : RenderPassCommandEncoder(command_buffer) {}

  void PipelineBarrier(PipelineStageFlag source_stage_mask,
                       PipelineStageFlag target_stage_mask,
                       PipelineDependencyFlag dependency_flags) override {}

  void MemoryBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask) override {}

  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer,
                     size_t offset, size_t length) override {}

  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image,
                    Image::LayerRange layer_range) override {}

  void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) override {}

  void ClearColorAttachment(int color_attachment_index, ClearColor clear_color,
                            ArrayView<ClearRect> clear_rects) override {}

  void ClearDepthStencilAttachment(float depth_value, uint32_t stencil_value,
                                   ArrayView<ClearRect> clear_rects) override {}

  void NextSubpass() override {}

  void SetScissors(int first_scissor, ArrayView<Rect2D> scissors) override {}

  void SetViewports(int first_viewport,
                    ArrayView<Viewport> viewports) override {}

  void SetLineWidth(float line_width) override {}

  void SetDepthBias(float depth_bias_constant_factor, float depth_bias_clamp,
                    float depth_bias_slope_factor) override {}

  void SetDepthBounds(float min_depth_bounds, float max_depth_bounds) override {
  }

  void SetStencilCompareMask(StencilFaceFlag face_mask,
                             uint32_t compare_mask) override {}

  void SetStencilWriteMask(StencilFaceFlag face_mask,
                           uint32_t write_mask) override {}

  void SetStencilReference(StencilFaceFlag face_mask,
                           uint32_t reference) override {}

  void SetBlendConstants(const float blend_constants[4]) override {}

  void BindPipeline(ref_ptr<RenderPipeline> pipeline) override {}

  void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set,
                       ArrayView<size_t> dynamic_offsets) override {}

  void PushConstants(ref_ptr<PipelineLayout> pipeline_layout,
                     ShaderStageFlag stage_mask, size_t offset,
                     const void* data, size_t data_length) override {}

  void BindVertexBuffers(int first_binding,
                         ArrayView<ref_ptr<Buffer>> buffers) override {}
  void BindVertexBuffers(int first_binding, ArrayView<ref_ptr<Buffer>> buffers,
                         ArrayView<size_t> buffer_offsets) override {}

  void BindIndexBuffer(ref_ptr<Buffer> buffer, size_t buffer_offset,
                       IndexElementType index_type) override {}

  void Draw(int vertex_count, int instance_count, int first_vertex,
            int first_instance) override {}

  void DrawIndexed(int index_count, int instance_count, int first_index,
                   int vertex_offset, int first_instance) override {}

  void DrawIndirect(ref_ptr<Buffer> buffer, size_t buffer_offset,
                    int draw_count, size_t stride) override {}

  void DrawIndexedIndirect(ref_ptr<Buffer> buffer, size_t buffer_offset,
                           int draw_count, size_t stride) override {}
};

}  // namespace testing
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_TESTING_PARTIAL_COMMAND_ENCODER_H_
