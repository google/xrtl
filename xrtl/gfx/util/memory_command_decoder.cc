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

#include "xrtl/gfx/util/memory_command_decoder.h"

#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/gfx/command_buffer.h"

namespace xrtl {
namespace gfx {
namespace util {

bool MemoryCommandDecoder::Decode(MemoryCommandBufferReader* reader,
                                  CommandBuffer* target_command_buffer) {
  CommandEncoder* any_encoder = nullptr;
  TransferCommandEncoder* any_transfer_encoder = nullptr;
  TransferCommandEncoderPtr transfer_encoder{nullptr, nullptr};
  ComputeCommandEncoderPtr compute_encoder{nullptr, nullptr};
  RenderCommandEncoderPtr render_encoder{nullptr, nullptr};
  RenderPassCommandEncoderPtr render_pass_encoder{nullptr, nullptr};

  while (!reader->empty()) {
    // Read command header.
    auto command_header = reader->PeekCommandHeader();
    if (!command_header) {
      LOG(ERROR) << "Error reading command buffer";
      return false;
    }

    // Giant switch for command dispatch.
    switch (command_header->command_type) {
      case CommandType::kBeginTransferCommands: {
        reader->ReadCommand<BeginTransferCommandsCommand>(command_header);
        transfer_encoder = target_command_buffer->BeginTransferCommands();
        any_transfer_encoder = transfer_encoder.get();
        any_encoder = transfer_encoder.get();
        break;
      }
      case CommandType::kEndTransferCommands: {
        reader->ReadCommand<EndTransferCommandsCommand>(command_header);
        target_command_buffer->EndTransferCommands(std::move(transfer_encoder));
        any_encoder = nullptr;
        any_transfer_encoder = nullptr;
        break;
      }
      case CommandType::kBeginComputeCommands: {
        reader->ReadCommand<BeginComputeCommandsCommand>(command_header);
        compute_encoder = target_command_buffer->BeginComputeCommands();
        any_encoder = compute_encoder.get();
        any_transfer_encoder = compute_encoder.get();
        break;
      }
      case CommandType::kEndComputeCommands: {
        reader->ReadCommand<EndComputeCommandsCommand>(command_header);
        target_command_buffer->EndComputeCommands(std::move(compute_encoder));
        any_encoder = nullptr;
        any_transfer_encoder = nullptr;
        break;
      }
      case CommandType::kBeginRenderCommands: {
        reader->ReadCommand<BeginRenderCommandsCommand>(command_header);
        render_encoder = target_command_buffer->BeginRenderCommands();
        any_encoder = render_encoder.get();
        any_transfer_encoder = render_encoder.get();
        break;
      }
      case CommandType::kEndRenderCommands: {
        reader->ReadCommand<EndRenderCommandsCommand>(command_header);
        target_command_buffer->EndRenderCommands(std::move(render_encoder));
        any_encoder = nullptr;
        any_transfer_encoder = nullptr;
        break;
      }
      case CommandType::kBeginRenderPass: {
        auto command =
            reader->ReadCommand<BeginRenderPassCommand>(command_header);
        render_pass_encoder = target_command_buffer->BeginRenderPass(
            ref_ptr<RenderPass>(command.render_pass),
            ref_ptr<Framebuffer>(command.framebuffer),
            reader->ReadArray<ClearColor>(command.clear_color_count));
        any_encoder = render_encoder.get();
        any_transfer_encoder = render_encoder.get();
        break;
      }
      case CommandType::kEndRenderPass: {
        reader->ReadCommand<EndRenderPassCommand>(command_header);
        target_command_buffer->EndRenderPass(std::move(render_pass_encoder));
        any_encoder = nullptr;
        any_transfer_encoder = nullptr;
        break;
      }

      case CommandType::kSetFence: {
        auto command = reader->ReadCommand<SetFenceCommand>(command_header);
        if (compute_encoder) {
          compute_encoder->SetFence(ref_ptr<CommandFence>(command.fence),
                                    command.pipeline_stage_mask);
        } else {
          render_encoder->SetFence(ref_ptr<CommandFence>(command.fence),
                                   command.pipeline_stage_mask);
        }
        break;
      }
      case CommandType::kResetFence: {
        auto command = reader->ReadCommand<ResetFenceCommand>(command_header);
        if (compute_encoder) {
          compute_encoder->ResetFence(ref_ptr<CommandFence>(command.fence),
                                      command.pipeline_stage_mask);
        } else {
          render_encoder->ResetFence(ref_ptr<CommandFence>(command.fence),
                                     command.pipeline_stage_mask);
        }
        break;
      }
      case CommandType::kWaitFences: {
        auto command = reader->ReadCommand<WaitFencesCommand>(command_header);
        auto fences =
            reader->ReadRefPtrArray<CommandFence>(command.fence_count);
        if (compute_encoder) {
          compute_encoder->WaitFences(fences);
        } else if (render_encoder) {
          render_encoder->WaitFences(fences);
        } else {
          render_pass_encoder->WaitFences(fences);
        }
        break;
      }

      case CommandType::kPipelineBarrier: {
        auto command =
            reader->ReadCommand<PipelineBarrierCommand>(command_header);
        any_encoder->PipelineBarrier(command.source_stage_mask,
                                     command.target_stage_mask,
                                     command.dependency_flags);
        break;
      }
      case CommandType::kMemoryBarrier: {
        auto command =
            reader->ReadCommand<MemoryBarrierCommand>(command_header);
        any_encoder->MemoryBarrier(
            command.source_stage_mask, command.target_stage_mask,
            command.dependency_flags, command.source_access_mask,
            command.target_access_mask);
        break;
      }
      case CommandType::kBufferBarrier: {
        auto command =
            reader->ReadCommand<BufferBarrierCommand>(command_header);
        any_encoder->BufferBarrier(
            command.source_stage_mask, command.target_stage_mask,
            command.dependency_flags, command.source_access_mask,
            command.target_access_mask, ref_ptr<Buffer>(command.buffer),
            command.offset, command.length);
        break;
      }
      case CommandType::kImageBarrier: {
        auto command = reader->ReadCommand<ImageBarrierCommand>(command_header);
        any_encoder->ImageBarrier(
            command.source_stage_mask, command.target_stage_mask,
            command.dependency_flags, command.source_access_mask,
            command.target_access_mask, command.source_layout,
            command.target_layout, ref_ptr<Image>(command.image),
            command.layer_range);
        break;
      }

      case CommandType::kFillBuffer: {
        auto command = reader->ReadCommand<FillBufferCommand>(command_header);
        any_transfer_encoder->FillBuffer(ref_ptr<Buffer>(command.buffer),
                                         command.offset, command.length,
                                         command.value);
        break;
      }
      case CommandType::kUpdateBuffer: {
        auto command = reader->ReadCommand<UpdateBufferCommand>(command_header);
        auto source_data = reader->ReadData(command.source_data_length);
        any_transfer_encoder->UpdateBuffer(
            ref_ptr<Buffer>(command.target_buffer), command.target_offset,
            source_data, command.source_data_length);
        break;
      }
      case CommandType::kCopyBuffer: {
        auto command = reader->ReadCommand<CopyBufferCommand>(command_header);
        auto regions =
            reader->ReadArray<CopyBufferRegion>(command.region_count);
        any_transfer_encoder->CopyBuffer(ref_ptr<Buffer>(command.source_buffer),
                                         ref_ptr<Buffer>(command.target_buffer),
                                         regions);
        break;
      }
      case CommandType::kCopyImage: {
        auto command = reader->ReadCommand<CopyImageCommand>(command_header);
        auto regions = reader->ReadArray<CopyImageRegion>(command.region_count);
        any_transfer_encoder->CopyImage(ref_ptr<Image>(command.source_image),
                                        command.source_image_layout,
                                        ref_ptr<Image>(command.target_image),
                                        command.target_image_layout, regions);
        break;
      }
      case CommandType::kCopyBufferToImage: {
        auto command =
            reader->ReadCommand<CopyBufferToImageCommand>(command_header);
        auto regions =
            reader->ReadArray<CopyBufferImageRegion>(command.region_count);
        any_transfer_encoder->CopyBufferToImage(
            ref_ptr<Buffer>(command.source_buffer),
            ref_ptr<Image>(command.target_image), command.target_image_layout,
            regions);
        break;
      }
      case CommandType::kCopyImageToBuffer: {
        auto command =
            reader->ReadCommand<CopyImageToBufferCommand>(command_header);
        auto regions =
            reader->ReadArray<CopyBufferImageRegion>(command.region_count);
        any_transfer_encoder->CopyImageToBuffer(
            ref_ptr<Image>(command.source_image), command.source_image_layout,
            ref_ptr<Buffer>(command.target_buffer), regions);
        break;
      }
      case CommandType::kBlitImage: {
        auto command = reader->ReadCommand<BlitImageCommand>(command_header);
        auto regions = reader->ReadArray<BlitImageRegion>(command.region_count);
        render_encoder->BlitImage(
            ref_ptr<Image>(command.source_image), command.source_image_layout,
            ref_ptr<Image>(command.target_image), command.target_image_layout,
            command.scaling_filter, regions);
        break;
      }
      case CommandType::kResolveImage: {
        auto command = reader->ReadCommand<ResolveImageCommand>(command_header);
        auto regions = reader->ReadArray<CopyImageRegion>(command.region_count);
        render_encoder->ResolveImage(ref_ptr<Image>(command.source_image),
                                     command.source_image_layout,
                                     ref_ptr<Image>(command.target_image),
                                     command.target_image_layout, regions);
        break;
      }
      case CommandType::kGenerateMipmaps: {
        auto command =
            reader->ReadCommand<GenerateMipmapsCommand>(command_header);
        render_encoder->GenerateMipmaps(ref_ptr<Image>(command.image));
        break;
      }

      case CommandType::kClearColorImage: {
        auto command =
            reader->ReadCommand<ClearColorImageCommand>(command_header);
        auto ranges = reader->ReadArray<Image::LayerRange>(command.range_count);
        if (compute_encoder) {
          compute_encoder->ClearColorImage(ref_ptr<Image>(command.image),
                                           command.image_layout,
                                           command.clear_color, ranges);
        } else {
          render_encoder->ClearColorImage(ref_ptr<Image>(command.image),
                                          command.image_layout,
                                          command.clear_color, ranges);
        }
        break;
      }
      case CommandType::kClearDepthStencilImage: {
        auto command =
            reader->ReadCommand<ClearDepthStencilImageCommand>(command_header);
        auto ranges = reader->ReadArray<Image::LayerRange>(command.range_count);
        render_encoder->ClearDepthStencilImage(
            ref_ptr<Image>(command.image), command.image_layout,
            command.depth_value, command.stencil_value, ranges);
        break;
      }
      case CommandType::kClearColorAttachment: {
        auto command =
            reader->ReadCommand<ClearColorAttachmentCommand>(command_header);
        auto clear_rects =
            reader->ReadArray<ClearRect>(command.clear_rect_count);
        render_pass_encoder->ClearColorAttachment(
            command.color_attachment_index, command.clear_color, clear_rects);
        break;
      }
      case CommandType::kClearDepthStencilAttachment: {
        auto command = reader->ReadCommand<ClearDepthStencilAttachmentCommand>(
            command_header);
        auto clear_rects =
            reader->ReadArray<ClearRect>(command.clear_rect_count);
        render_pass_encoder->ClearDepthStencilAttachment(
            command.depth_value, command.stencil_value, clear_rects);
        break;
      }

      case CommandType::kBindComputePipeline: {
        auto command =
            reader->ReadCommand<BindComputePipelineCommand>(command_header);
        compute_encoder->BindPipeline(
            ref_ptr<ComputePipeline>(command.pipeline));
        break;
      }
      case CommandType::kBindRenderPipeline: {
        auto command =
            reader->ReadCommand<BindRenderPipelineCommand>(command_header);
        render_pass_encoder->BindPipeline(
            ref_ptr<RenderPipeline>(command.pipeline));
        break;
      }
      case CommandType::kBindResourceSet: {
        auto command =
            reader->ReadCommand<BindResourceSetCommand>(command_header);
        auto dynamic_offsets =
            reader->ReadArray<size_t>(command.dynamic_offset_count);
        if (compute_encoder) {
          compute_encoder->BindResourceSet(
              command.set_index, ref_ptr<ResourceSet>(command.resource_set),
              dynamic_offsets);
        } else {
          render_pass_encoder->BindResourceSet(
              command.set_index, ref_ptr<ResourceSet>(command.resource_set),
              dynamic_offsets);
        }
        break;
      }
      case CommandType::kPushConstants: {
        auto command =
            reader->ReadCommand<PushConstantsCommand>(command_header);
        auto data = reader->ReadData(command.data_length);
        if (compute_encoder) {
          compute_encoder->PushConstants(
              ref_ptr<PipelineLayout>(command.pipeline_layout),
              command.stage_mask, command.offset, data, command.data_length);
        } else {
          render_pass_encoder->PushConstants(
              ref_ptr<PipelineLayout>(command.pipeline_layout),
              command.stage_mask, command.offset, data, command.data_length);
        }
        break;
      }

      case CommandType::kDispatch: {
        auto command = reader->ReadCommand<DispatchCommand>(command_header);
        compute_encoder->Dispatch(command.group_count_x, command.group_count_y,
                                  command.group_count_z);
        break;
      }
      case CommandType::kDispatchIndirect: {
        auto command =
            reader->ReadCommand<DispatchIndirectCommand>(command_header);
        compute_encoder->DispatchIndirect(ref_ptr<Buffer>(command.buffer),
                                          command.offset);
        break;
      }

      case CommandType::kNextSubpass: {
        reader->ReadCommand<NextSubpassCommand>(command_header);
        render_pass_encoder->NextSubpass();
        break;
      }
      case CommandType::kSetScissors: {
        auto command = reader->ReadCommand<SetScissorsCommand>(command_header);
        auto scissors = reader->ReadArray<Rect2D>(command.scissor_count);
        render_pass_encoder->SetScissors(command.first_scissor, scissors);
        break;
      }
      case CommandType::kSetViewports: {
        auto command = reader->ReadCommand<SetViewportsCommand>(command_header);
        auto viewports = reader->ReadArray<Viewport>(command.viewport_count);
        render_pass_encoder->SetViewports(command.first_viewport, viewports);
        break;
      }
      case CommandType::kSetLineWidth: {
        auto command = reader->ReadCommand<SetLineWidthCommand>(command_header);
        render_pass_encoder->SetLineWidth(command.line_width);
        break;
      }
      case CommandType::kSetDepthBias: {
        auto command = reader->ReadCommand<SetDepthBiasCommand>(command_header);
        render_pass_encoder->SetDepthBias(command.depth_bias_constant_factor,
                                          command.depth_bias_clamp,
                                          command.depth_bias_slope_factor);
        break;
      }
      case CommandType::kSetDepthBounds: {
        auto command =
            reader->ReadCommand<SetDepthBoundsCommand>(command_header);
        render_pass_encoder->SetDepthBounds(command.min_depth_bounds,
                                            command.max_depth_bounds);
        break;
      }
      case CommandType::kSetStencilCompareMask: {
        auto command =
            reader->ReadCommand<SetStencilCompareMaskCommand>(command_header);
        render_pass_encoder->SetStencilCompareMask(command.face_mask,
                                                   command.compare_mask);
        break;
      }
      case CommandType::kSetStencilWriteMask: {
        auto command =
            reader->ReadCommand<SetStencilWriteMaskCommand>(command_header);
        render_pass_encoder->SetStencilWriteMask(command.face_mask,
                                                 command.write_mask);
        break;
      }
      case CommandType::kSetStencilReference: {
        auto command =
            reader->ReadCommand<SetStencilReferenceCommand>(command_header);
        render_pass_encoder->SetStencilReference(command.face_mask,
                                                 command.reference);
        break;
      }
      case CommandType::kSetBlendConstants: {
        auto command =
            reader->ReadCommand<SetBlendConstantsCommand>(command_header);
        render_pass_encoder->SetBlendConstants(command.blend_constants);
        break;
      }
      case CommandType::kBindVertexBuffers: {
        auto command =
            reader->ReadCommand<BindVertexBuffersCommand>(command_header);
        auto buffers = reader->ReadRefPtrArray<Buffer>(command.buffer_count);
        if (command.has_offsets) {
          auto buffer_offsets = reader->ReadArray<size_t>(command.buffer_count);
          render_pass_encoder->BindVertexBuffers(command.first_binding, buffers,
                                                 buffer_offsets);
        } else {
          render_pass_encoder->BindVertexBuffers(command.first_binding,
                                                 buffers);
        }
        break;
      }
      case CommandType::kBindIndexBuffer: {
        auto command =
            reader->ReadCommand<BindIndexBufferCommand>(command_header);
        render_pass_encoder->BindIndexBuffer(ref_ptr<Buffer>(command.buffer),
                                             command.buffer_offset,
                                             command.index_type);
        break;
      }
      case CommandType::kDraw: {
        auto command = reader->ReadCommand<DrawCommand>(command_header);
        render_pass_encoder->Draw(command.vertex_count, command.instance_count,
                                  command.first_vertex, command.first_instance);
        break;
      }
      case CommandType::kDrawIndexed: {
        auto command = reader->ReadCommand<DrawIndexedCommand>(command_header);
        render_pass_encoder->DrawIndexed(
            command.index_count, command.instance_count, command.first_index,
            command.vertex_offset, command.first_instance);
        break;
      }
      case CommandType::kDrawIndirect: {
        auto command = reader->ReadCommand<DrawIndirectCommand>(command_header);
        render_pass_encoder->DrawIndirect(ref_ptr<Buffer>(command.buffer),
                                          command.buffer_offset,
                                          command.draw_count, command.stride);
        break;
      }
      case CommandType::kDrawIndexedIndirect: {
        auto command =
            reader->ReadCommand<DrawIndexedIndirectCommand>(command_header);
        render_pass_encoder->DrawIndexedIndirect(
            ref_ptr<Buffer>(command.buffer), command.buffer_offset,
            command.draw_count, command.stride);
        break;
      }

      default: {
        LOG(FATAL) << "Unknown command type";
        break;
      }
    }
  }

  return true;
}

}  // namespace util
}  // namespace gfx
}  // namespace xrtl
