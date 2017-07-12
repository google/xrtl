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

#ifndef XRTL_GFX_COMMAND_ENCODER_H_
#define XRTL_GFX_COMMAND_ENCODER_H_

#include <memory>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/buffer.h"
#include "xrtl/gfx/command_fence.h"
#include "xrtl/gfx/image.h"
#include "xrtl/gfx/pipeline.h"
#include "xrtl/gfx/pipeline_layout.h"
#include "xrtl/gfx/resource_set.h"
#include "xrtl/gfx/sampler.h"

namespace xrtl {
namespace gfx {

class CommandBuffer;

// A bitmask specifying the set of stencil state for which to update.
enum class StencilFaceFlag : uint32_t {
  // Indicates that only the front set of stencil state is updated.
  kFaceFront = 1 << 0,
  // Indicates that only the back set of stencil state is updated.
  kFaceBack = 1 << 1,
  // Indicates that both sets of stencil state are updated.
  kFrontAndBack = kFaceFront | kFaceBack,
};
XRTL_BITMASK(StencilFaceFlag);

// Defines a buffer copy region.
struct CopyBufferRegion {
  size_t source_offset = 0;
  size_t target_offset = 0;
  size_t length = 0;
};

// Defines an image copy region.
struct CopyImageRegion {
  Image::LayerRange source_layer_range;
  Point3D source_offset;
  Image::LayerRange target_layer_range;
  Point3D target_offset;
  Size3D size;
};

// Defines a buffer-image copy region.
struct CopyBufferImageRegion {
  size_t buffer_offset;
  int buffer_row_length;
  int buffer_image_height;
  Image::LayerRange image_layer_range;
  Rect3D image_rect;
};

// Defines an image blit source and target region.
struct BlitImageRegion {
  Image::LayerRange source_layer_range;
  Rect3D source_rect;
  Image::LayerRange target_layer_range;
  Rect3D target_rect;
};

// Defines a clear rectangle.
struct ClearRect {
  // 2D region to be cleared.
  Rect2D rect;
  // Starting layer index to clear.
  int base_layer = 0;
  // Total number of layers to clear.
  int layer_count = 1;

  ClearRect(int x, int y, int width, int height) : rect(x, y, width, height) {}
  explicit ClearRect(Rect2D rect) : rect(rect) {}
  ClearRect(Rect2D rect, int base_layer, int layer_count)
      : rect(rect), base_layer(base_layer), layer_count(layer_count) {}
};

// Defines a clear color value.
// The type the value is interpreted as depends on the target buffer.
union ClearColor {
  float float_value[4];
  int32_t sint_value[4];
  uint32_t uint_value[4];

  ClearColor() = default;
  ClearColor(float r, float g, float b, float a) {
    float_value[0] = r;
    float_value[1] = g;
    float_value[2] = b;
    float_value[3] = a;
  }
};

// Defines a fixed-function viewport.
struct Viewport {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
  float min_depth = 0.0f;
  float max_depth = 1.0f;

  Viewport() = default;
  Viewport(float x, float y, float width, float height)
      : x(x), y(y), width(width), height(height) {}
  Viewport(Point2D origin, Size2D size)
      : x(static_cast<float>(origin.x)),
        y(static_cast<float>(origin.y)),
        width(static_cast<float>(size.width)),
        height(static_cast<float>(size.height)) {}
  explicit Viewport(Rect2D rect)
      : x(static_cast<float>(rect.origin.x)),
        y(static_cast<float>(rect.origin.y)),
        width(static_cast<float>(rect.size.width)),
        height(static_cast<float>(rect.size.height)) {}
};

// Defines the index buffer element type size.
enum class IndexElementType {
  // Unsigned 16-bit integer indices.
  // When primitive restart is enabled the index 0xFFFF is reserved.
  kUint16,
  // Unsigned 32-bit integer indices.
  // When primitive restart is enabled the index 0xFFFFFFFF is reserved.
  kUint32,
};

// Base command encoder.
// See one of the specific encoders for more information.
class CommandEncoder {
 public:
  // The command buffer the encoder is encoding into.
  CommandBuffer* command_buffer() const { return command_buffer_; }

  // Inserts a dependency between two stages of the pipeline.
  // This will split commands encoded into the command buffer based on where in
  // the stages they fall. Ordering is always preserved. All commands executed
  // in the source stages are guaranteed to complete before any commands in the
  // target stages execute.
  //
  // Queue: any.
  virtual void PipelineBarrier(PipelineStageFlag source_stage_mask,
                               PipelineStageFlag target_stage_mask,
                               PipelineDependencyFlag dependency_flags) = 0;

  // Inserts a memory dependency between two stages of the pipeline.
  // Memory accesses using the set of access types in source_access_mask
  // performed in pipeline stages in source_stage_mask by the first set of
  // commands must complete and be available to later commands. The side effects
  // of the first set of commands will be visible to memory accesses using the
  // set of access types in target_access_mask performed in pipeline stages in
  // target_stage_mask by the second set of commands.
  //
  // If the barrier is framebuffer-local these requirements only apply to
  // invocations within the same framebuffer-space region and for pipeline
  // stages that perform framebuffer-space work.
  //
  // The execution dependency guarantees that execution of work by the
  // destination stages of the second set of commands will not begin until
  // execution of work by the source stages of the first set of commands has
  // completed.
  //
  // Queue: any.
  virtual void MemoryBarrier(PipelineStageFlag source_stage_mask,
                             PipelineStageFlag target_stage_mask,
                             PipelineDependencyFlag dependency_flags,
                             AccessFlag source_access_mask,
                             AccessFlag target_access_mask) = 0;

  // Inserts a memory dependency between two stages of the pipeline.
  // This type of barrier only applies to memory accesses involving a specific
  // range of the specified buffer object. That is, a memory dependency formed
  // from a buffer memory barrier is scoped to the specified range of the
  // buffer.
  //
  // Queue: any.
  virtual void BufferBarrier(PipelineStageFlag source_stage_mask,
                             PipelineStageFlag target_stage_mask,
                             PipelineDependencyFlag dependency_flags,
                             AccessFlag source_access_mask,
                             AccessFlag target_access_mask,
                             ref_ptr<Buffer> buffer, size_t offset,
                             size_t length) = 0;
  void BufferBarrier(PipelineStageFlag source_stage_mask,
                     PipelineStageFlag target_stage_mask,
                     PipelineDependencyFlag dependency_flags,
                     AccessFlag source_access_mask,
                     AccessFlag target_access_mask, ref_ptr<Buffer> buffer) {
    BufferBarrier(source_stage_mask, target_stage_mask, dependency_flags,
                  source_access_mask, target_access_mask, buffer, 0,
                  buffer->allocation_size());
  }

  // TODO(benvanik): image regions.
  // Inserts a memory dependency between two stages of the pipeline.
  // This type of barrier only applies to memory accesses involving a specific
  // image subresource range of the specified image object. That is, a
  // memory dependency formed from an image memory barrier is scoped to the
  // specified image subresources of the image. It is also used to perform a
  // layout transition for an image subresource range.
  //
  // source_layout may be Layout::kUndefined if it is not known. target_layout
  // must not be Layout::kUndefined.
  //
  // Queue: any.
  virtual void ImageBarrier(PipelineStageFlag source_stage_mask,
                            PipelineStageFlag target_stage_mask,
                            PipelineDependencyFlag dependency_flags,
                            AccessFlag source_access_mask,
                            AccessFlag target_access_mask,
                            Image::Layout source_layout,
                            Image::Layout target_layout, ref_ptr<Image> image,
                            Image::LayerRange layer_range) = 0;
  void ImageBarrier(PipelineStageFlag source_stage_mask,
                    PipelineStageFlag target_stage_mask,
                    PipelineDependencyFlag dependency_flags,
                    AccessFlag source_access_mask,
                    AccessFlag target_access_mask, Image::Layout source_layout,
                    Image::Layout target_layout, ref_ptr<Image> image) {
    ImageBarrier(source_stage_mask, target_stage_mask, dependency_flags,
                 source_access_mask, target_access_mask, source_layout,
                 target_layout, image, image->entire_range());
  }

  // TODO(benvanik): API for transfering queue ownership to enable multi-queue.
  // virtual void TransferBufferQueue(...) = 0;
  // virtual void TransferImageQueue(...) = 0;

 protected:
  explicit CommandEncoder(CommandBuffer* command_buffer)
      : command_buffer_(command_buffer) {}

  CommandBuffer* command_buffer_ = nullptr;
};

// Command encoder for transfer commands.
// Transfer commands deal with manipulating buffers and images in a way that
// can often run in parallel with compute and render commands. Transfer commands
// cannot perform any format conversion or work with data that may be packed in
// a device-specific format (so clearing depth buffers isn't possible).
class TransferCommandEncoder : public CommandEncoder {
 public:
  // Fills a buffer with a repeating data value.
  // This can be used to quickly clear a buffer.
  // The size passed must be 4-byte aligned. If it is not aligned then the size
  // will be rounded down to the next smallest 4-byte interval.
  //
  // Queue: transfer.
  // Stage: transfer.
  //
  // target_buffer must have Usage::kTransferTarget.
  virtual void FillBuffer(ref_ptr<Buffer> buffer, size_t offset, size_t length,
                          uint8_t value) = 0;

  // Updates buffer contents inline from the command buffer.
  // This can be faster (and significantly easier) for updating small buffers,
  // though it should be used sparingly as to not bloat command buffers.
  // The data size is limited to 65536 bytes (64k). For larger updates use real
  // buffer upload techniques like MapMemory or staging buffers.
  //
  // Queue: transfer.
  // Stage: transfer.
  //
  // target_buffer must have Usage::kTransferTarget.
  virtual void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                            const void* source_data,
                            size_t source_data_length) = 0;
  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const std::vector<uint8_t>& source_data) {
    return UpdateBuffer(std::move(target_buffer), target_offset,
                        source_data.data(), source_data.size());
  }
  void UpdateBuffer(ref_ptr<Buffer> target_buffer, size_t target_offset,
                    const std::vector<uint8_t>& source_data,
                    size_t source_data_offset, size_t source_data_length) {
    return UpdateBuffer(std::move(target_buffer), target_offset,
                        source_data.data() + source_data_offset,
                        source_data_length);
  }

  // Copies data from one buffer to another.
  // The source and target buffer may be the same (alias), but just as with
  // memcpy the regions must not overlap.
  //
  // Queue: transfer.
  // Stage: transfer.
  //
  // source_buffer must have Usage::kTransferSource.
  // target_buffer must have Usage::kTransferTarget.
  virtual void CopyBuffer(ref_ptr<Buffer> source_buffer,
                          ref_ptr<Buffer> target_buffer,
                          ArrayView<CopyBufferRegion> regions) = 0;

  // Copies data between two images without performing conversion.
  // This is effectively a memcpy, and as such cannot scale/resize/convert the
  // image contents. The source and target images may be the same (alias),
  // but just as with memcpy the regions must not overlap.
  //
  // The source and target images must be either the same format or a
  // compatible format. Formats are compatible if their element size is the same
  // (such as kR8G8B8A8 and kR32, which are both 4-byte elements). Depth/stencil
  // formats must match exactly.
  //
  // When copying to/from or between compressed formats the extents provided
  // in the regions must be multiples of the compressed texel block sizes.
  //
  // For more details see vkCmdCopyImage:
  // https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkCmdCopyImage.html
  //
  // Queue: transfer.
  // Stage: transfer.
  //
  // source_image must have Usage::kTransferSource.
  // source_image_layout must be kGeneral or kTransferSourceOptimal.
  // target_image must have Usage::kTransferTarget.
  // target_image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void CopyImage(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         ArrayView<CopyImageRegion> regions) = 0;

  // Copies data from a buffer to an image.
  //
  // Queue: transfer.
  // Stage: transfer.
  //
  // source_buffer must have Usage::kTransferSource.
  // target_image must have Usage::kTransferTarget.
  // target_image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void CopyBufferToImage(ref_ptr<Buffer> source_buffer,
                                 ref_ptr<Image> target_image,
                                 Image::Layout target_image_layout,
                                 ArrayView<CopyBufferImageRegion> regions) = 0;

  // Copies data from an image to a buffer.
  //
  // Queue: transfer.
  // Stage: transfer.
  //
  // source_image must have Usage::kTransferSource.
  // source_image_layout must be kGeneral or kTransferSourceOptimal.
  // target_buffer must have Usage::kTransferTarget.
  virtual void CopyImageToBuffer(ref_ptr<Image> source_image,
                                 Image::Layout source_image_layout,
                                 ref_ptr<Buffer> target_buffer,
                                 ArrayView<CopyBufferImageRegion> regions) = 0;

 protected:
  explicit TransferCommandEncoder(CommandBuffer* command_buffer)
      : CommandEncoder(command_buffer) {}
};

using TransferCommandEncoderPtr =
    std::unique_ptr<TransferCommandEncoder, void (*)(TransferCommandEncoder*)>;

// Command encoder for compute commands.
// Everything required to fully execute compute pipelines can be encoded here.
// Compute commands may be able to run on their own queue in parallel with
// transfer or render commands.
//
// Some platforms may not support compute pipelines and they should be feature
// detected before attempting to encode command buffers with them.
class ComputeCommandEncoder : public TransferCommandEncoder {
 public:
  // Sets a command fence to signaled state.
  // The fence will be signaled after all commands previously encoded that
  // affect the given stages complete.
  // Fences may only be signaled once and repeated SetFence calls will be
  // no-ops.
  virtual void SetFence(ref_ptr<CommandFence> fence,
                        PipelineStageFlag pipeline_stage_mask) = 0;

  // Resets a fence object to non-signaled state.
  // The fence will be reset after all commands previously encoded that affect
  // the given stages complete.
  // Fences may only be reset once and repeated ResetFence calls will be no-ops.
  virtual void ResetFence(ref_ptr<CommandFence> fence,
                          PipelineStageFlag pipeline_stage_mask) = 0;

  // Waits for the given fence to be signaled.
  // If it is already signaled the wait will continue immediately.
  // This is usually followed by one or more barriers to ensure memory safety.
  virtual void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) = 0;
  void WaitFence(ref_ptr<CommandFence> fence) { WaitFences({fence}); }

  // Clears regions of a color image.
  //
  // Queue: compute.
  //
  // image must have Usage::kTransferTarget.
  // image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                               ClearColor clear_color,
                               ArrayView<Image::LayerRange> ranges) = 0;

  // Binds a pipeline object to a command buffer.
  // All future compute dispatches will use this pipeline until another is
  // bound.
  //
  // Queue: compute.
  virtual void BindPipeline(ref_ptr<ComputePipeline> pipeline) = 0;

  // Binds a pipeline binding set to a command buffer at the given index.
  // All future compute dispatches will use the bound set.
  // If the resource set contains kUniformBufferDynamic or kStorageBufferDynamic
  // slots the dynamic_offsets array should provide offsets for those slots. The
  // order is the same as the slots in the pipeline layout.
  //
  // Queue: compute.
  virtual void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set,
                               ArrayView<size_t> dynamic_offsets) = 0;
  void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set) {
    BindResourceSet(set_index, std::move(resource_set), {});
  }

  // Updates the values of push constants.
  // The stage_mask specifies which shader stages will use the updated values.
  //
  // Queue: compute.
  virtual void PushConstants(ref_ptr<PipelineLayout> pipeline_layout,
                             ShaderStageFlag stage_mask, size_t offset,
                             const void* data, size_t data_length) = 0;

  // Dispatches compute work items.
  // The maximum group counts are specified in Device::Limits.
  //
  // Queue: compute.
  virtual void Dispatch(int group_count_x, int group_count_y,
                        int group_count_z) = 0;

  // Dispatchs compute work items using indirect parameters.
  //
  // Queue: compute.
  virtual void DispatchIndirect(ref_ptr<Buffer> buffer, size_t offset) = 0;

 protected:
  explicit ComputeCommandEncoder(CommandBuffer* command_buffer)
      : TransferCommandEncoder(command_buffer) {}
};

using ComputeCommandEncoderPtr =
    std::unique_ptr<ComputeCommandEncoder, void (*)(ComputeCommandEncoder*)>;

// Command encoder for generic render commands.
// These commands run on the render queue but happen outside of a render pass.
// For commands related to drawing see the RenderPassCommandEncoder which
// encodes drawing-specific commands.
class RenderCommandEncoder : public TransferCommandEncoder {
 public:
  // Sets a command fence to signaled state.
  // The fence will be signaled after all commands previously encoded that
  // affect the given stages complete.
  // Fences may only be signaled once and repeated SetFence calls will be
  // no-ops.
  virtual void SetFence(ref_ptr<CommandFence> fence,
                        PipelineStageFlag pipeline_stage_mask) = 0;

  // Resets a fence object to non-signaled state.
  // The fence will be reset after all commands previously encoded that affect
  // the given stages complete.
  // Fences may only be reset once and repeated ResetFence calls will be no-ops.
  virtual void ResetFence(ref_ptr<CommandFence> fence,
                          PipelineStageFlag pipeline_stage_mask) = 0;

  // Waits for the given fence to be signaled.
  // If it is already signaled the wait will continue immediately.
  // This is usually followed by one or more barriers to ensure memory safety.
  virtual void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) = 0;
  void WaitFence(ref_ptr<CommandFence> fence) { WaitFences({fence}); }

  // Clears regions of a color image.
  //
  // Queue: render.
  //
  // image must have Usage::kTransferTarget.
  // image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void ClearColorImage(ref_ptr<Image> image, Image::Layout image_layout,
                               ClearColor clear_color,
                               ArrayView<Image::LayerRange> ranges) = 0;

  // Fills regions of a combined depth/stencil image.
  //
  // Queue: render.
  //
  // image must have Usage::kTransferTarget.
  // image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void ClearDepthStencilImage(ref_ptr<Image> image,
                                      Image::Layout image_layout,
                                      float depth_value, uint32_t stencil_value,
                                      ArrayView<Image::LayerRange> ranges) = 0;

  // Copies regions of an image potentially performing format conversion.
  // There are tons of restrictions on this. See the reference.
  //
  // For more details see vkCmdBlitImage:
  // https://www.khronos.org/registry/vulkan/specs/1.0/man/html/vkCmdBlitImage.html
  //
  // Queue: render.
  //
  // source_image must have Usage::kTransferSource.
  // source_image_layout must be kGeneral or kTransferSourceOptimal.
  // target_image must have Usage::kTransferTarget.
  // target_image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void BlitImage(ref_ptr<Image> source_image,
                         Image::Layout source_image_layout,
                         ref_ptr<Image> target_image,
                         Image::Layout target_image_layout,
                         Sampler::Filter scaling_filter,
                         ArrayView<BlitImageRegion> regions) = 0;

  // Resolves regions of a multisample image to a non-multisample image.
  //
  // Queue: render.
  //
  // source_image_layout must be kGeneral or kTransferSourceOptimal.
  // target_image_layout must be kGeneral or kTransferTargetOptimal.
  virtual void ResolveImage(ref_ptr<Image> source_image,
                            Image::Layout source_image_layout,
                            ref_ptr<Image> target_image,
                            Image::Layout target_image_layout,
                            ArrayView<CopyImageRegion> regions) = 0;

  // TODO(benvanik): specify layer range?
  // Generates mipmaps for the given image.
  // Mip level 0 will be used to populate the entire mip chain for all layers.
  // The image must have been created with mip levels. Existing contents will
  // be overwritten.
  //
  // Queue: render.
  //
  // TODO(benvanik): restrictions on layout/usage.
  virtual void GenerateMipmaps(ref_ptr<Image> image) = 0;

 protected:
  explicit RenderCommandEncoder(CommandBuffer* command_buffer)
      : TransferCommandEncoder(command_buffer) {}
};

using RenderCommandEncoderPtr =
    std::unique_ptr<RenderCommandEncoder, void (*)(RenderCommandEncoder*)>;

// Command encoder for render passes.
// All encoded commands are performed within the context of the render pass that
// was used to create the encoder. If the render pass contains multiple
// subpasses the NextSubpass method must be used to advance through all of them
// during encoding.
class RenderPassCommandEncoder : public CommandEncoder {
 public:
  // Waits for the given fence to be signaled.
  // If it is already signaled the wait will continue immediately.
  // This is usually followed by one or more barriers to ensure memory safety.
  virtual void WaitFences(ArrayView<ref_ptr<CommandFence>> fences) = 0;
  void WaitFence(ref_ptr<CommandFence> fence) { WaitFences({fence}); }

  // Clears one or more regions of color attachments inside a render pass.
  // The attachment must be active in the current subpass.
  //
  // Queue: render.
  virtual void ClearColorAttachment(int color_attachment_index,
                                    ClearColor clear_color,
                                    ArrayView<ClearRect> clear_rects) = 0;

  // Clears one or more regions of a depth/stencil attachment inside a render
  // pass. The current subpass must have a depth/stencil attachment.
  //
  // Queue: render.
  virtual void ClearDepthStencilAttachment(
      float depth_value, uint32_t stencil_value,
      ArrayView<ClearRect> clear_rects) = 0;

  // Transitions to the next sub pass in the render pass.
  //
  // Queue: render.
  virtual void NextSubpass() = 0;

  // Sets the dynamic scissor rectangles on a command buffer.
  //
  // Queue: render
  virtual void SetScissors(int first_scissor, ArrayView<Rect2D> scissors) = 0;
  void SetScissor(Rect2D rect) { SetScissors(0, {rect}); }

  // Sets the viewports on a command buffer.
  //
  // Queue: render.
  virtual void SetViewports(int first_viewport,
                            ArrayView<Viewport> viewports) = 0;
  void SetViewport(Viewport viewport) { SetViewports(0, {viewport}); }
  void SetViewport(Point2D origin, Size2D size) {
    SetViewport(Viewport{origin, size});
  }
  void SetViewport(Size2D size) { SetViewport(Viewport{{0, 0}, size}); }
  void SetViewport(Rect2D rect) { SetViewport(Viewport{rect}); }

  // Sets the dynamic line width state.
  //
  // Queue: render
  virtual void SetLineWidth(float line_width) = 0;

  // Sets the depth bias dynamic state.
  //
  // Queue: render
  virtual void SetDepthBias(float depth_bias_constant_factor,
                            float depth_bias_clamp,
                            float depth_bias_slope_factor) = 0;

  // Sets the depth bounds test values for a command buffer.
  //
  // Queue: render
  virtual void SetDepthBounds(float min_depth_bounds,
                              float max_depth_bounds) = 0;

  // Sets the stencil compare mask dynamic state.
  //
  // Queue: render
  virtual void SetStencilCompareMask(StencilFaceFlag face_mask,
                                     uint32_t compare_mask) = 0;

  // Sets the stencil write mask dynamic state.
  //
  // Queue: render
  virtual void SetStencilWriteMask(StencilFaceFlag face_mask,
                                   uint32_t write_mask) = 0;

  // Sets the stencil reference dynamic state.
  //
  // Queue: render
  virtual void SetStencilReference(StencilFaceFlag face_mask,
                                   uint32_t reference) = 0;

  // Sets the values of blend constants.
  //
  // Queue: render
  virtual void SetBlendConstants(const float blend_constants[4]) = 0;

  // Binds a pipeline object to a command buffer.
  // All future draws will use this pipeline until another is bound.
  //
  // Queue: render.
  virtual void BindPipeline(ref_ptr<RenderPipeline> pipeline) = 0;

  // Binds a pipeline binding set to a command buffer at the given index.
  // All future draws will use the bound set.
  //
  // Queue: render.
  virtual void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set,
                               ArrayView<size_t> dynamic_offsets) = 0;
  void BindResourceSet(int set_index, ref_ptr<ResourceSet> resource_set) {
    BindResourceSet(set_index, std::move(resource_set), {});
  }

  // Updates the values of push constants.
  // The stage_mask specifies which shader stages will use the updated values.
  //
  // Queue: render.
  virtual void PushConstants(ref_ptr<PipelineLayout> pipeline_layout,
                             ShaderStageFlag stage_mask, size_t offset,
                             const void* data, size_t data_length) = 0;

  // Binds vertex buffers to a command buffer.
  //
  // Queue: render.
  virtual void BindVertexBuffers(int first_binding,
                                 ArrayView<ref_ptr<Buffer>> buffers) = 0;
  virtual void BindVertexBuffers(int first_binding,
                                 ArrayView<ref_ptr<Buffer>> buffers,
                                 ArrayView<size_t> buffer_offsets) = 0;

  // Binds an index buffer to a command buffer.
  //
  // Queue: render.
  virtual void BindIndexBuffer(ref_ptr<Buffer> buffer, size_t buffer_offset,
                               IndexElementType index_type) = 0;

  // Issues a non-indexed draw into a command buffer.
  //
  // Queue: render.
  virtual void Draw(int vertex_count, int instance_count, int first_vertex,
                    int first_instance) = 0;
  void Draw(int vertex_count) { Draw(vertex_count, 1, 0, 0); }

  // Issues an indexed draw into a command buffer.
  //
  // Queue: render.
  virtual void DrawIndexed(int index_count, int instance_count, int first_index,
                           int vertex_offset, int first_instance) = 0;
  void DrawIndexed(int index_count) { DrawIndexed(index_count, 1, 0, 0, 0); }

  // Issues an indirect non-indexed draw into a command buffer.
  // draw_count parameter sets are read from the buffer and issued.
  // stride is the distance between successive sets of draw parameters.
  //
  // Queue: render.
  virtual void DrawIndirect(ref_ptr<Buffer> buffer, size_t buffer_offset,
                            int draw_count, size_t stride) = 0;

  // Issues an indirect indexed draw into a command buffer.
  // draw_count parameter sets are read from the buffer and issued.
  // stride is the distance between successive sets of draw parameters.
  //
  // Queue: render.
  virtual void DrawIndexedIndirect(ref_ptr<Buffer> buffer, size_t buffer_offset,
                                   int draw_count, size_t stride) = 0;

 protected:
  explicit RenderPassCommandEncoder(CommandBuffer* command_buffer)
      : CommandEncoder(command_buffer) {}
};

using RenderPassCommandEncoderPtr =
    std::unique_ptr<RenderPassCommandEncoder,
                    void (*)(RenderPassCommandEncoder*)>;

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_COMMAND_ENCODER_H_
