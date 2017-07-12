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

#ifndef XRTL_GFX_UTIL_MEMORY_COMMANDS_H_
#define XRTL_GFX_UTIL_MEMORY_COMMANDS_H_

#include "xrtl/gfx/command_encoder.h"
#include "xrtl/gfx/framebuffer.h"

namespace xrtl {
namespace gfx {
namespace util {

// This is an in-memory protocol: it contains pointers to current process
// memory. This means that you cannot serialize this to disk and expect to load
// it back in a way that correctly references memory (it is possible, however,
// to build an indirection system to do such a thing).
//
// The protocol is *not* secure. It should not be used on untrusted data.
//
// All structs are splated directly into buffers and effectively memcpy'ed -
// this means that all data must be POD - a ref_ptr or std::string will not be
// correctly manipulated!
//
// Commands that have variable sizes (such as those accepting data buffers or
// arrays) are encoded with a count in their command struct and then immediately
// followed by the encoded variable-length data.
//
// The command buffer writes to a slab-style arena allocator with some defined
// block size (that is a reasonable max of any command + data we'll encode).
// Each block from the allocator is prefixed with a kBufferPacketFrame command
// denoting the total size of the packet and a pointer to the next packet.

// Maximum size of a single command in bytes.
// This is used to size buffers that hold the commands.
// Note that the 64k here comes from UpdateBuffer's max inline data size.
constexpr size_t kMaxCommandSize = (4 + 64) * 1024;

enum class CommandType {
  kBeginTransferCommands = 0,
  kEndTransferCommands,
  kBeginComputeCommands,
  kEndComputeCommands,
  kBeginRenderCommands,
  kEndRenderCommands,
  kBeginRenderPass,
  kEndRenderPass,

  kSetFence,
  kResetFence,
  kWaitFences,
  kPipelineBarrier,
  kMemoryBarrier,
  kBufferBarrier,
  kImageBarrier,
  kFillBuffer,
  kUpdateBuffer,
  kCopyBuffer,
  kCopyImage,
  kCopyBufferToImage,
  kCopyImageToBuffer,
  kBlitImage,
  kResolveImage,
  kGenerateMipmaps,
  kClearColorImage,
  kClearDepthStencilImage,
  kClearColorAttachment,
  kClearDepthStencilAttachment,
  kBindComputePipeline,
  kBindRenderPipeline,
  kBindResourceSet,
  kPushConstants,
  kDispatch,
  kDispatchIndirect,
  kNextSubpass,
  kSetScissors,
  kSetViewports,
  kSetLineWidth,
  kSetDepthBias,
  kSetDepthBounds,
  kSetStencilCompareMask,
  kSetStencilWriteMask,
  kSetStencilReference,
  kSetBlendConstants,
  kBindVertexBuffers,
  kBindIndexBuffer,
  kDraw,
  kDrawIndexed,
  kDrawIndirect,
  kDrawIndexedIndirect,
};

// Command packet header used by the buffer reader/writer.
struct PacketHeader {
  size_t packet_length;       // Excludes this header.
  PacketHeader* next_packet;  // nullptr if this is the last packet.
};

// Single command header that prefixes all commands written.
struct CommandHeader {
  CommandType command_type;
};

struct BeginTransferCommandsCommand {};
struct EndTransferCommandsCommand {};
struct BeginComputeCommandsCommand {};
struct EndComputeCommandsCommand {};
struct BeginRenderCommandsCommand {};
struct EndRenderCommandsCommand {};

struct BeginRenderPassCommand {
  RenderPass* render_pass;
  Framebuffer* framebuffer;
  size_t clear_color_count;
};

struct EndRenderPassCommand {};

struct SetFenceCommand {
  CommandFence* fence;
  PipelineStageFlag pipeline_stage_mask;
};

struct ResetFenceCommand {
  CommandFence* fence;
  PipelineStageFlag pipeline_stage_mask;
};

struct WaitFencesCommand {
  size_t fence_count;
  // ArrayView<CommandFence*> fences;
};

struct PipelineBarrierCommand {
  PipelineStageFlag source_stage_mask;
  PipelineStageFlag target_stage_mask;
  PipelineDependencyFlag dependency_flags;
};

struct MemoryBarrierCommand {
  PipelineStageFlag source_stage_mask;
  PipelineStageFlag target_stage_mask;
  PipelineDependencyFlag dependency_flags;
  AccessFlag source_access_mask;
  AccessFlag target_access_mask;
};

struct BufferBarrierCommand {
  PipelineStageFlag source_stage_mask;
  PipelineStageFlag target_stage_mask;
  PipelineDependencyFlag dependency_flags;
  AccessFlag source_access_mask;
  AccessFlag target_access_mask;
  Buffer* buffer;
  size_t offset;
  size_t length;
};

struct ImageBarrierCommand {
  PipelineStageFlag source_stage_mask;
  PipelineStageFlag target_stage_mask;
  PipelineDependencyFlag dependency_flags;
  AccessFlag source_access_mask;
  AccessFlag target_access_mask;
  Image::Layout source_layout;
  Image::Layout target_layout;
  Image* image;
  Image::LayerRange layer_range;
};

struct FillBufferCommand {
  Buffer* buffer;
  size_t offset;
  size_t length;
  uint8_t value;
};

struct UpdateBufferCommand {
  Buffer* target_buffer;
  size_t target_offset;
  size_t source_data_length;
  // const void* source_data;
};

struct CopyBufferCommand {
  Buffer* source_buffer;
  Buffer* target_buffer;
  size_t region_count;
  // ArrayView<CopyBufferRegion> regions;
};

struct CopyImageCommand {
  Image* source_image;
  Image::Layout source_image_layout;
  Image* target_image;
  Image::Layout target_image_layout;
  size_t region_count;
  // ArrayView<CopyImageRegion> regions;
};

struct CopyBufferToImageCommand {
  Buffer* source_buffer;
  Image* target_image;
  Image::Layout target_image_layout;
  size_t region_count;
  // ArrayView<CopyBufferImageRegion> regions;
};

struct CopyImageToBufferCommand {
  Image* source_image;
  Image::Layout source_image_layout;
  Buffer* target_buffer;
  size_t region_count;
  // ArrayView<CopyBufferImageRegion> regions;
};

struct BlitImageCommand {
  Image* source_image;
  Image::Layout source_image_layout;
  Image* target_image;
  Image::Layout target_image_layout;
  Sampler::Filter scaling_filter;
  size_t region_count;
  // ArrayView<BlitImageRegion> regions;
};

struct ResolveImageCommand {
  Image* source_image;
  Image::Layout source_image_layout;
  Image* target_image;
  Image::Layout target_image_layout;
  size_t region_count;
  // ArrayView<CopyImageRegion> regions;
};

struct GenerateMipmapsCommand {
  Image* image;
};

struct ClearColorImageCommand {
  Image* image;
  Image::Layout image_layout;
  ClearColor clear_color;
  size_t range_count;
  // ArrayView<Image::LayerRange> ranges;
};

struct ClearDepthStencilImageCommand {
  Image* image;
  Image::Layout image_layout;
  float depth_value;
  uint32_t stencil_value;
  size_t range_count;
  // ArrayView<Image::LayerRange> ranges;
};

struct ClearColorAttachmentCommand {
  int color_attachment_index;
  ClearColor clear_color;
  size_t clear_rect_count;
  // ArrayView<ClearRect> clear_rects;
};

struct ClearDepthStencilAttachmentCommand {
  float depth_value;
  uint32_t stencil_value;
  size_t clear_rect_count;
  // ArrayView<ClearRect> clear_rects;
};

struct BindComputePipelineCommand {
  ComputePipeline* pipeline;
};

struct BindRenderPipelineCommand {
  RenderPipeline* pipeline;
};

struct BindResourceSetCommand {
  int set_index;
  ResourceSet* resource_set;
  size_t dynamic_offset_count;
  // ArrayView<size_t> dynamic_offsets;
};

struct PushConstantsCommand {
  PipelineLayout* pipeline_layout;
  ShaderStageFlag stage_mask;
  size_t offset;
  size_t data_length;
  // const void* data;
};

struct DispatchCommand {
  int group_count_x;
  int group_count_y;
  int group_count_z;
};

struct DispatchIndirectCommand {
  Buffer* buffer;
  size_t offset;
};

struct NextSubpassCommand {};

struct SetScissorsCommand {
  int first_scissor;
  size_t scissor_count;
  // ArrayView<Rect2D> scissors;
};

struct SetViewportsCommand {
  int first_viewport;
  size_t viewport_count;
  // ArrayView<Viewport> viewports;
};

struct SetLineWidthCommand {
  float line_width;
};

struct SetDepthBiasCommand {
  float depth_bias_constant_factor;
  float depth_bias_clamp;
  float depth_bias_slope_factor;
};

struct SetDepthBoundsCommand {
  float min_depth_bounds;
  float max_depth_bounds;
};

struct SetStencilCompareMaskCommand {
  StencilFaceFlag face_mask;
  uint32_t compare_mask;
};

struct SetStencilWriteMaskCommand {
  StencilFaceFlag face_mask;
  uint32_t write_mask;
};

struct SetStencilReferenceCommand {
  StencilFaceFlag face_mask;
  uint32_t reference;
};

struct SetBlendConstantsCommand {
  float blend_constants[4];
};

struct BindVertexBuffersCommand {
  int first_binding;
  bool has_offsets;
  size_t buffer_count;
  // ArrayView<Buffer*> buffers;
  // if has_offsets:
  //   ArrayView<size_t> buffer_offsets;
};

struct BindIndexBufferCommand {
  Buffer* buffer;
  size_t buffer_offset;
  IndexElementType index_type;
};

struct DrawCommand {
  int vertex_count;
  int instance_count;
  int first_vertex;
  int first_instance;
};

struct DrawIndexedCommand {
  int index_count;
  int instance_count;
  int first_index;
  int vertex_offset;
  int first_instance;
};

struct DrawIndirectCommand {
  Buffer* buffer;
  size_t buffer_offset;
  int draw_count;
  size_t stride;
};

struct DrawIndexedIndirectCommand {
  Buffer* buffer;
  size_t buffer_offset;
  int draw_count;
  size_t stride;
};

}  // namespace util
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_UTIL_MEMORY_COMMANDS_H_
