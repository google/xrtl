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

#include "xrtl/gfx/util/memory_command_encoder.h"

#include "xrtl/gfx/command_buffer.h"

namespace xrtl {
namespace gfx {
namespace util {

MemoryTransferCommandEncoder::MemoryTransferCommandEncoder(
    CommandBuffer* command_buffer, MemoryCommandBufferWriter* writer)
    : TransferCommandEncoder(command_buffer), writer_(writer) {}

MemoryTransferCommandEncoder::~MemoryTransferCommandEncoder() = default;

void MemoryTransferCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  writer_->WriteCommand(
      CommandType::kPipelineBarrier,
      PipelineBarrierCommand{source_stage_mask, target_stage_mask,
                             dependency_flags});
}

void MemoryTransferCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  writer_->WriteCommand(
      CommandType::kMemoryBarrier,
      MemoryBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask});
}

void MemoryTransferCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(
      CommandType::kBufferBarrier,
      BufferBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask, buffer.get(), offset, length});
}

void MemoryTransferCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(
      CommandType::kImageBarrier,
      ImageBarrierCommand{source_stage_mask, target_stage_mask,
                          dependency_flags, source_access_mask,
                          target_access_mask, source_layout, target_layout,
                          image.get(), layer_range});
}

void MemoryTransferCommandEncoder::FillBuffer(ref_ptr<Buffer> buffer,
                                              size_t offset, size_t length,
                                              uint8_t value) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(CommandType::kFillBuffer,
                        FillBufferCommand{buffer.get(), offset, length, value});
}

void MemoryTransferCommandEncoder::UpdateBuffer(ref_ptr<Buffer> target_buffer,
                                                size_t target_offset,
                                                const void* source_data,
                                                size_t source_data_length) {
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(CommandType::kUpdateBuffer,
                        UpdateBufferCommand{target_buffer.get(), target_offset,
                                            source_data_length});
  writer_->WriteData(source_data, source_data_length);
}

void MemoryTransferCommandEncoder::CopyBuffer(
    ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
    ArrayView<CopyBufferRegion> regions) {
  command_buffer_->AttachDependency(source_buffer);
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(CommandType::kCopyBuffer,
                        CopyBufferCommand{source_buffer.get(),
                                          target_buffer.get(), regions.size()});
  writer_->WriteArray(regions);
}

void MemoryTransferCommandEncoder::CopyImage(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Image> target_image, Image::Layout target_image_layout,
    ArrayView<CopyImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kCopyImage,
      CopyImageCommand{source_image.get(), source_image_layout,
                       target_image.get(), target_image_layout,
                       regions.size()});
  writer_->WriteArray(regions);
}

void MemoryTransferCommandEncoder::CopyBufferToImage(
    ref_ptr<Buffer> source_buffer, ref_ptr<Image> target_image,
    Image::Layout target_image_layout,
    ArrayView<CopyBufferImageRegion> regions) {
  command_buffer_->AttachDependency(source_buffer);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kCopyBufferToImage,
      CopyBufferToImageCommand{source_buffer.get(), target_image.get(),
                               target_image_layout, regions.size()});
  writer_->WriteArray(regions);
}

void MemoryTransferCommandEncoder::CopyImageToBuffer(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Buffer> target_buffer, ArrayView<CopyBufferImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(
      CommandType::kCopyImageToBuffer,
      CopyImageToBufferCommand{source_image.get(), source_image_layout,
                               target_buffer.get(), regions.size()});
  writer_->WriteArray(regions);
}

MemoryComputeCommandEncoder::MemoryComputeCommandEncoder(
    CommandBuffer* command_buffer, MemoryCommandBufferWriter* writer)
    : ComputeCommandEncoder(command_buffer), writer_(writer) {}

MemoryComputeCommandEncoder::~MemoryComputeCommandEncoder() = default;

void MemoryComputeCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  writer_->WriteCommand(
      CommandType::kPipelineBarrier,
      PipelineBarrierCommand{source_stage_mask, target_stage_mask,
                             dependency_flags});
}

void MemoryComputeCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  writer_->WriteCommand(
      CommandType::kMemoryBarrier,
      MemoryBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask});
}

void MemoryComputeCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(
      CommandType::kBufferBarrier,
      BufferBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask, buffer.get(), offset, length});
}

void MemoryComputeCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(
      CommandType::kImageBarrier,
      ImageBarrierCommand{source_stage_mask, target_stage_mask,
                          dependency_flags, source_access_mask,
                          target_access_mask, source_layout, target_layout,
                          image.get(), layer_range});
}

void MemoryComputeCommandEncoder::FillBuffer(ref_ptr<Buffer> buffer,
                                             size_t offset, size_t length,
                                             uint8_t value) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(CommandType::kFillBuffer,
                        FillBufferCommand{buffer.get(), offset, length, value});
}

void MemoryComputeCommandEncoder::UpdateBuffer(ref_ptr<Buffer> target_buffer,
                                               size_t target_offset,
                                               const void* source_data,
                                               size_t source_data_length) {
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(CommandType::kUpdateBuffer,
                        UpdateBufferCommand{target_buffer.get(), target_offset,
                                            source_data_length});
  writer_->WriteData(source_data, source_data_length);
}

void MemoryComputeCommandEncoder::CopyBuffer(
    ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
    ArrayView<CopyBufferRegion> regions) {
  command_buffer_->AttachDependency(source_buffer);
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(CommandType::kCopyBuffer,
                        CopyBufferCommand{source_buffer.get(),
                                          target_buffer.get(), regions.size()});
  writer_->WriteArray(regions);
}

void MemoryComputeCommandEncoder::CopyImage(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Image> target_image, Image::Layout target_image_layout,
    ArrayView<CopyImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kCopyImage,
      CopyImageCommand{source_image.get(), source_image_layout,
                       target_image.get(), target_image_layout,
                       regions.size()});
  writer_->WriteArray(regions);
}

void MemoryComputeCommandEncoder::CopyBufferToImage(
    ref_ptr<Buffer> source_buffer, ref_ptr<Image> target_image,
    Image::Layout target_image_layout,
    ArrayView<CopyBufferImageRegion> regions) {
  command_buffer_->AttachDependency(source_buffer);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kCopyBufferToImage,
      CopyBufferToImageCommand{source_buffer.get(), target_image.get(),
                               target_image_layout, regions.size()});
  writer_->WriteArray(regions);
}

void MemoryComputeCommandEncoder::CopyImageToBuffer(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Buffer> target_buffer, ArrayView<CopyBufferImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(
      CommandType::kCopyImageToBuffer,
      CopyImageToBufferCommand{source_image.get(), source_image_layout,
                               target_buffer.get(), regions.size()});
  writer_->WriteArray(regions);
}

void MemoryComputeCommandEncoder::SetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  command_buffer_->AttachDependency(fence);
  writer_->WriteCommand(CommandType::kSetFence,
                        SetFenceCommand{fence.get(), pipeline_stage_mask});
}

void MemoryComputeCommandEncoder::ResetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  command_buffer_->AttachDependency(fence);
  writer_->WriteCommand(CommandType::kResetFence,
                        ResetFenceCommand{fence.get(), pipeline_stage_mask});
}

void MemoryComputeCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  command_buffer_->AttachDependencies(fences);
  writer_->WriteCommand(CommandType::kWaitFences,
                        WaitFencesCommand{fences.size()});
  writer_->WriteArray(fences);
}

void MemoryComputeCommandEncoder::ClearColorImage(
    ref_ptr<Image> image, Image::Layout image_layout, ClearColor clear_color,
    ArrayView<Image::LayerRange> ranges) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(CommandType::kClearColorImage,
                        ClearColorImageCommand{image.get(), image_layout,
                                               clear_color, ranges.size()});
  writer_->WriteArray(ranges);
}

void MemoryComputeCommandEncoder::BindPipeline(
    ref_ptr<ComputePipeline> pipeline) {
  command_buffer_->AttachDependency(pipeline);
  writer_->WriteCommand(CommandType::kBindComputePipeline,
                        BindComputePipelineCommand{pipeline.get()});
}

void MemoryComputeCommandEncoder::BindResourceSet(
    int set_index, ref_ptr<ResourceSet> resource_set,
    ArrayView<size_t> dynamic_offsets) {
  command_buffer_->AttachDependency(resource_set);
  writer_->WriteCommand(CommandType::kBindResourceSet,
                        BindResourceSetCommand{set_index, resource_set.get(),
                                               dynamic_offsets.size()});
  writer_->WriteArray(dynamic_offsets);
}

void MemoryComputeCommandEncoder::PushConstants(
    ref_ptr<PipelineLayout> pipeline_layout, ShaderStageFlag stage_mask,
    size_t offset, const void* data, size_t data_length) {
  command_buffer_->AttachDependency(pipeline_layout);
  writer_->WriteCommand(CommandType::kPushConstants,
                        PushConstantsCommand{pipeline_layout.get(), stage_mask,
                                             offset, data_length});
  writer_->WriteData(data, data_length);
}

void MemoryComputeCommandEncoder::Dispatch(int group_count_x, int group_count_y,
                                           int group_count_z) {
  writer_->WriteCommand(
      CommandType::kDispatch,
      DispatchCommand{group_count_x, group_count_y, group_count_z});
}

void MemoryComputeCommandEncoder::DispatchIndirect(ref_ptr<Buffer> buffer,
                                                   size_t offset) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(CommandType::kDispatchIndirect,
                        DispatchIndirectCommand{buffer.get(), offset});
}

MemoryRenderCommandEncoder::MemoryRenderCommandEncoder(
    CommandBuffer* command_buffer, MemoryCommandBufferWriter* writer)
    : RenderCommandEncoder(command_buffer), writer_(writer) {}

MemoryRenderCommandEncoder::~MemoryRenderCommandEncoder() = default;

void MemoryRenderCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  writer_->WriteCommand(
      CommandType::kPipelineBarrier,
      PipelineBarrierCommand{source_stage_mask, target_stage_mask,
                             dependency_flags});
}

void MemoryRenderCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  writer_->WriteCommand(
      CommandType::kMemoryBarrier,
      MemoryBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask});
}

void MemoryRenderCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(
      CommandType::kBufferBarrier,
      BufferBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask, buffer.get(), offset, length});
}

void MemoryRenderCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(
      CommandType::kImageBarrier,
      ImageBarrierCommand{source_stage_mask, target_stage_mask,
                          dependency_flags, source_access_mask,
                          target_access_mask, source_layout, target_layout,
                          image.get(), layer_range});
}

void MemoryRenderCommandEncoder::FillBuffer(ref_ptr<Buffer> buffer,
                                            size_t offset, size_t length,
                                            uint8_t value) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(CommandType::kFillBuffer,
                        FillBufferCommand{buffer.get(), offset, length, value});
}

void MemoryRenderCommandEncoder::UpdateBuffer(ref_ptr<Buffer> target_buffer,
                                              size_t target_offset,
                                              const void* source_data,
                                              size_t source_data_length) {
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(CommandType::kUpdateBuffer,
                        UpdateBufferCommand{target_buffer.get(), target_offset,
                                            source_data_length});
  writer_->WriteData(source_data, source_data_length);
}

void MemoryRenderCommandEncoder::CopyBuffer(
    ref_ptr<Buffer> source_buffer, ref_ptr<Buffer> target_buffer,
    ArrayView<CopyBufferRegion> regions) {
  command_buffer_->AttachDependency(source_buffer);
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(CommandType::kCopyBuffer,
                        CopyBufferCommand{source_buffer.get(),
                                          target_buffer.get(), regions.size()});
  writer_->WriteArray(regions);
}

void MemoryRenderCommandEncoder::CopyImage(ref_ptr<Image> source_image,
                                           Image::Layout source_image_layout,
                                           ref_ptr<Image> target_image,
                                           Image::Layout target_image_layout,
                                           ArrayView<CopyImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kCopyImage,
      CopyImageCommand{source_image.get(), source_image_layout,
                       target_image.get(), target_image_layout,
                       regions.size()});
  writer_->WriteArray(regions);
}

void MemoryRenderCommandEncoder::CopyBufferToImage(
    ref_ptr<Buffer> source_buffer, ref_ptr<Image> target_image,
    Image::Layout target_image_layout,
    ArrayView<CopyBufferImageRegion> regions) {
  command_buffer_->AttachDependency(source_buffer);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kCopyBufferToImage,
      CopyBufferToImageCommand{source_buffer.get(), target_image.get(),
                               target_image_layout, regions.size()});
  writer_->WriteArray(regions);
}

void MemoryRenderCommandEncoder::CopyImageToBuffer(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Buffer> target_buffer, ArrayView<CopyBufferImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_buffer);
  writer_->WriteCommand(
      CommandType::kCopyImageToBuffer,
      CopyImageToBufferCommand{source_image.get(), source_image_layout,
                               target_buffer.get(), regions.size()});
  writer_->WriteArray(regions);
}

void MemoryRenderCommandEncoder::SetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  command_buffer_->AttachDependency(fence);
  writer_->WriteCommand(CommandType::kSetFence,
                        SetFenceCommand{fence.get(), pipeline_stage_mask});
}

void MemoryRenderCommandEncoder::ResetFence(
    ref_ptr<CommandFence> fence, PipelineStageFlag pipeline_stage_mask) {
  command_buffer_->AttachDependency(fence);
  writer_->WriteCommand(CommandType::kResetFence,
                        ResetFenceCommand{fence.get(), pipeline_stage_mask});
}

void MemoryRenderCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  command_buffer_->AttachDependencies(fences);
  writer_->WriteCommand(CommandType::kWaitFences,
                        WaitFencesCommand{fences.size()});
  writer_->WriteArray(fences);
}

void MemoryRenderCommandEncoder::ClearColorImage(
    ref_ptr<Image> image, Image::Layout image_layout, ClearColor clear_color,
    ArrayView<Image::LayerRange> ranges) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(CommandType::kClearColorImage,
                        ClearColorImageCommand{image.get(), image_layout,
                                               clear_color, ranges.size()});
  writer_->WriteArray(ranges);
}

void MemoryRenderCommandEncoder::ClearDepthStencilImage(
    ref_ptr<Image> image, Image::Layout image_layout, float depth_value,
    uint32_t stencil_value, ArrayView<Image::LayerRange> ranges) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(
      CommandType::kClearDepthStencilImage,
      ClearDepthStencilImageCommand{image.get(), image_layout, depth_value,
                                    stencil_value, ranges.size()});
  writer_->WriteArray(ranges);
}

void MemoryRenderCommandEncoder::BlitImage(ref_ptr<Image> source_image,
                                           Image::Layout source_image_layout,
                                           ref_ptr<Image> target_image,
                                           Image::Layout target_image_layout,
                                           Sampler::Filter scaling_filter,
                                           ArrayView<BlitImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kBlitImage,
      BlitImageCommand{source_image.get(), source_image_layout,
                       target_image.get(), target_image_layout, scaling_filter,
                       regions.size()});
  writer_->WriteArray(regions);
}

void MemoryRenderCommandEncoder::ResolveImage(
    ref_ptr<Image> source_image, Image::Layout source_image_layout,
    ref_ptr<Image> target_image, Image::Layout target_image_layout,
    ArrayView<CopyImageRegion> regions) {
  command_buffer_->AttachDependency(source_image);
  command_buffer_->AttachDependency(target_image);
  writer_->WriteCommand(
      CommandType::kResolveImage,
      ResolveImageCommand{source_image.get(), source_image_layout,
                          target_image.get(), target_image_layout,
                          regions.size()});
  writer_->WriteArray(regions);
}

void MemoryRenderCommandEncoder::GenerateMipmaps(ref_ptr<Image> image) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(CommandType::kGenerateMipmaps,
                        GenerateMipmapsCommand{image.get()});
}

MemoryRenderPassCommandEncoder::MemoryRenderPassCommandEncoder(
    CommandBuffer* command_buffer, MemoryCommandBufferWriter* writer)
    : RenderPassCommandEncoder(command_buffer), writer_(writer) {}

MemoryRenderPassCommandEncoder::~MemoryRenderPassCommandEncoder() = default;

void MemoryRenderPassCommandEncoder::PipelineBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags) {
  writer_->WriteCommand(
      CommandType::kPipelineBarrier,
      PipelineBarrierCommand{source_stage_mask, target_stage_mask,
                             dependency_flags});
}

void MemoryRenderPassCommandEncoder::MemoryBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask) {
  writer_->WriteCommand(
      CommandType::kMemoryBarrier,
      MemoryBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask});
}

void MemoryRenderPassCommandEncoder::BufferBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, ref_ptr<Buffer> buffer, size_t offset,
    size_t length) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(
      CommandType::kBufferBarrier,
      BufferBarrierCommand{source_stage_mask, target_stage_mask,
                           dependency_flags, source_access_mask,
                           target_access_mask, buffer.get(), offset, length});
}

void MemoryRenderPassCommandEncoder::ImageBarrier(
    PipelineStageFlag source_stage_mask, PipelineStageFlag target_stage_mask,
    PipelineDependencyFlag dependency_flags, AccessFlag source_access_mask,
    AccessFlag target_access_mask, Image::Layout source_layout,
    Image::Layout target_layout, ref_ptr<Image> image,
    Image::LayerRange layer_range) {
  command_buffer_->AttachDependency(image);
  writer_->WriteCommand(
      CommandType::kImageBarrier,
      ImageBarrierCommand{source_stage_mask, target_stage_mask,
                          dependency_flags, source_access_mask,
                          target_access_mask, source_layout, target_layout,
                          image.get(), layer_range});
}

void MemoryRenderPassCommandEncoder::WaitFences(
    ArrayView<ref_ptr<CommandFence>> fences) {
  command_buffer_->AttachDependencies(fences);
  writer_->WriteCommand(CommandType::kWaitFences,
                        WaitFencesCommand{fences.size()});
  writer_->WriteArray(fences);
}

void MemoryRenderPassCommandEncoder::ClearColorAttachment(
    int color_attachment_index, ClearColor clear_color,
    ArrayView<ClearRect> clear_rects) {
  writer_->WriteCommand(
      CommandType::kClearColorAttachment,
      ClearColorAttachmentCommand{color_attachment_index, clear_color,
                                  clear_rects.size()});
  writer_->WriteArray(clear_rects);
}

void MemoryRenderPassCommandEncoder::ClearDepthStencilAttachment(
    float depth_value, uint32_t stencil_value,
    ArrayView<ClearRect> clear_rects) {
  writer_->WriteCommand(CommandType::kClearDepthStencilAttachment,
                        ClearDepthStencilAttachmentCommand{
                            depth_value, stencil_value, clear_rects.size()});
  writer_->WriteArray(clear_rects);
}

void MemoryRenderPassCommandEncoder::NextSubpass() {
  writer_->WriteCommand(CommandType::kNextSubpass, NextSubpassCommand{});
}

void MemoryRenderPassCommandEncoder::SetScissors(int first_scissor,
                                                 ArrayView<Rect2D> scissors) {
  writer_->WriteCommand(CommandType::kSetScissors,
                        SetScissorsCommand{first_scissor, scissors.size()});
  writer_->WriteArray(scissors);
}

void MemoryRenderPassCommandEncoder::SetViewports(
    int first_viewport, ArrayView<Viewport> viewports) {
  writer_->WriteCommand(CommandType::kSetViewports,
                        SetViewportsCommand{first_viewport, viewports.size()});
  writer_->WriteArray(viewports);
}

void MemoryRenderPassCommandEncoder::SetLineWidth(float line_width) {
  writer_->WriteCommand(CommandType::kSetLineWidth,
                        SetLineWidthCommand{line_width});
}

void MemoryRenderPassCommandEncoder::SetDepthBias(
    float depth_bias_constant_factor, float depth_bias_clamp,
    float depth_bias_slope_factor) {
  writer_->WriteCommand(
      CommandType::kSetDepthBias,
      SetDepthBiasCommand{depth_bias_constant_factor, depth_bias_clamp,
                          depth_bias_slope_factor});
}

void MemoryRenderPassCommandEncoder::SetDepthBounds(float min_depth_bounds,
                                                    float max_depth_bounds) {
  writer_->WriteCommand(
      CommandType::kSetDepthBounds,
      SetDepthBoundsCommand{min_depth_bounds, max_depth_bounds});
}

void MemoryRenderPassCommandEncoder::SetStencilCompareMask(
    StencilFaceFlag face_mask, uint32_t compare_mask) {
  writer_->WriteCommand(CommandType::kSetStencilCompareMask,
                        SetStencilCompareMaskCommand{face_mask, compare_mask});
}

void MemoryRenderPassCommandEncoder::SetStencilWriteMask(
    StencilFaceFlag face_mask, uint32_t write_mask) {
  writer_->WriteCommand(CommandType::kSetStencilWriteMask,
                        SetStencilWriteMaskCommand{face_mask, write_mask});
}

void MemoryRenderPassCommandEncoder::SetStencilReference(
    StencilFaceFlag face_mask, uint32_t reference) {
  writer_->WriteCommand(CommandType::kSetStencilReference,
                        SetStencilReferenceCommand{face_mask, reference});
}

void MemoryRenderPassCommandEncoder::SetBlendConstants(
    const float blend_constants[4]) {
  writer_->WriteCommand(
      CommandType::kSetBlendConstants,
      SetBlendConstantsCommand{{blend_constants[0], blend_constants[1],
                                blend_constants[2], blend_constants[3]}});
}

void MemoryRenderPassCommandEncoder::BindPipeline(
    ref_ptr<RenderPipeline> pipeline) {
  command_buffer_->AttachDependency(pipeline);
  writer_->WriteCommand(CommandType::kBindRenderPipeline,
                        BindRenderPipelineCommand{pipeline.get()});
}

void MemoryRenderPassCommandEncoder::BindResourceSet(
    int set_index, ref_ptr<ResourceSet> resource_set,
    ArrayView<size_t> dynamic_offsets) {
  command_buffer_->AttachDependency(resource_set);
  writer_->WriteCommand(CommandType::kBindResourceSet,
                        BindResourceSetCommand{set_index, resource_set.get(),
                                               dynamic_offsets.size()});
  writer_->WriteArray(dynamic_offsets);
}

void MemoryRenderPassCommandEncoder::PushConstants(
    ref_ptr<PipelineLayout> pipeline_layout, ShaderStageFlag stage_mask,
    size_t offset, const void* data, size_t data_length) {
  command_buffer_->AttachDependency(pipeline_layout);
  writer_->WriteCommand(CommandType::kPushConstants,
                        PushConstantsCommand{pipeline_layout.get(), stage_mask,
                                             offset, data_length});
  writer_->WriteData(data, data_length);
}

void MemoryRenderPassCommandEncoder::BindVertexBuffers(
    int first_binding, ArrayView<ref_ptr<Buffer>> buffers) {
  command_buffer_->AttachDependencies(buffers);
  writer_->WriteCommand(
      CommandType::kBindVertexBuffers,
      BindVertexBuffersCommand{first_binding, false, buffers.size()});
  writer_->WriteArray(buffers);
}

void MemoryRenderPassCommandEncoder::BindVertexBuffers(
    int first_binding, ArrayView<ref_ptr<Buffer>> buffers,
    ArrayView<size_t> buffer_offsets) {
  command_buffer_->AttachDependencies(buffers);
  writer_->WriteCommand(
      CommandType::kBindVertexBuffers,
      BindVertexBuffersCommand{first_binding, true, buffers.size()});
  writer_->WriteArray(buffers);
  writer_->WriteArray(buffer_offsets);
}

void MemoryRenderPassCommandEncoder::BindIndexBuffer(
    ref_ptr<Buffer> buffer, size_t buffer_offset, IndexElementType index_type) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(
      CommandType::kBindIndexBuffer,
      BindIndexBufferCommand{buffer.get(), buffer_offset, index_type});
}

void MemoryRenderPassCommandEncoder::Draw(int vertex_count, int instance_count,
                                          int first_vertex,
                                          int first_instance) {
  writer_->WriteCommand(
      CommandType::kDraw,
      DrawCommand{vertex_count, instance_count, first_vertex, first_instance});
}

void MemoryRenderPassCommandEncoder::DrawIndexed(int index_count,
                                                 int instance_count,
                                                 int first_index,
                                                 int vertex_offset,
                                                 int first_instance) {
  writer_->WriteCommand(
      CommandType::kDrawIndexed,
      DrawIndexedCommand{index_count, instance_count, first_index,
                         vertex_offset, first_instance});
}

void MemoryRenderPassCommandEncoder::DrawIndirect(ref_ptr<Buffer> buffer,
                                                  size_t buffer_offset,
                                                  int draw_count,
                                                  size_t stride) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(
      CommandType::kDrawIndirect,
      DrawIndirectCommand{buffer.get(), buffer_offset, draw_count, stride});
}

void MemoryRenderPassCommandEncoder::DrawIndexedIndirect(ref_ptr<Buffer> buffer,
                                                         size_t buffer_offset,
                                                         int draw_count,
                                                         size_t stride) {
  command_buffer_->AttachDependency(buffer);
  writer_->WriteCommand(CommandType::kDrawIndexedIndirect,
                        DrawIndexedIndirectCommand{buffer.get(), buffer_offset,
                                                   draw_count, stride});
}

}  // namespace util
}  // namespace gfx
}  // namespace xrtl
