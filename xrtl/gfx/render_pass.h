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

#ifndef XRTL_GFX_RENDER_PASS_H_
#define XRTL_GFX_RENDER_PASS_H_

#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/image.h"
#include "xrtl/gfx/pixel_format.h"
#include "xrtl/gfx/render_state.h"

namespace xrtl {
namespace gfx {

// A bitmask specifying pipeline stage flags.
enum class PipelineStageFlag : uint32_t {
  // Stage of the pipeline where commands are initially received by the queue.
  // Queue: any.
  kTopOfPipe = 1 << 0,
  // Stage of the pipeline where Draw/DispatchIndirect data structures are
  // consumed.
  // Queue: kRender or kCompute.
  kDrawIndirect = 1 << 1,
  // Stage of the pipeline where vertex and index buffers are consumed.
  // Queue: kRender.
  kVertexInput = 1 << 2,
  // Vertex shader stage.
  // Queue: kRender.
  kVertexShader = 1 << 3,
  // Tessellation control shader stage.
  // Queue: kRender.
  kTessellationControlShader = 1 << 4,
  // Tessellation evaluation shader stage.
  // Queue: kRender.
  kTessellationEvaluationShader = 1 << 5,
  // Geometry shader stage.
  // Queue: kRender.
  kGeometryShader = 1 << 6,
  // Fragment shader stage.
  // Queue: kRender.
  kFragmentShader = 1 << 7,
  // Stage of the pipeline where early fragment tests (depth and stencil tests
  // before fragment shading) are performed.
  // Queue: kRender.
  kEarlyFragmentTests = 1 << 8,
  // Stage of the pipeline where late fragment tests (depth and stencil tests
  // after fragment shading) are performed.
  // Queue: kRender.
  kLateFragmentTests = 1 << 9,
  // Stage of the pipeline after blending where the final color values are
  // output from the pipeline. This stage also includes resolve operations that
  // occur at the end of a subpass. Note that this does not necessarily indicate
  // that the values have been committed to memory.
  // Queue: kRender.
  kColorAttachmentOutput = 1 << 10,
  // Execution of a compute shader.
  // Queue: kCompute.
  kComputeShader = 1 << 11,
  // Execution of copy commands. This includes the operations resulting from all
  // transfer commands. The set of transfer commands comprises:
  //   CopyBuffer
  //   CopyImage
  //   BlitImage
  //   CopyBufferToImage
  //   CopyImageToBuffer
  //   UpdateBuffer
  //   FillBuffer
  //   ClearColorImage
  //   ClearDepthStencilImage
  //   ResolveImage
  //   CopyQueryPoolResults
  // Queue: any.
  kTransfer = 1 << 12,
  // Final stage in the pipeline where commands complete execution.
  // Queue: any.
  kBottomOfPipe = 1 << 13,
  // A pseudo-stage indicating execution on the host of reads/writes of device
  // memory.
  // Queue: any.
  kHost = 1 << 14,
  // Execution of all graphics pipeline stages.
  // Queue: kRender.
  kAllGraphics = 1 << 15,
  // Execution of all stages supported on the queue.
  // Queue: any.
  kAllCommands = 1 << 16,
};
XRTL_BITMASK(PipelineStageFlag);

// A bitmask specifying a pipeline stage.
enum class ShaderStageFlag : uint32_t {
  kNone = 0,
  kVertex = 1 << 0,
  kTessellationControl = 1 << 1,
  kTessellationEvaluation = 1 << 2,
  kGeometry = 1 << 3,
  kFragment = 1 << 4,
  kCompute = 1 << 5,
  kAllGraphics = 1 << 6,
  kAll = 1 << 7,
};
XRTL_BITMASK(ShaderStageFlag);

// A bitmask specifying pipeline dependencies.
enum class PipelineDependencyFlag : uint32_t {
  kNone = 0,
  // Dependencies will be framebuffer-local (as opposed to framebuffer-global).
  // Framebuffer local dependencies are significantly more performant on tiled
  // renderers as global barriers require a full flush back to main memory.
  kFramebufferLocal = 1 << 0,
};
XRTL_BITMASK(PipelineDependencyFlag);

// A bitmask specifying the pipeline access flags.
enum class AccessFlag : uint32_t {
  // Indicates that the access is an indirect command structure read as part of
  // an indirect drawing command.
  // Queue: kRender or kCompute.
  kIndirectCommandRead = 1 << 0,
  // Indicates that the access is an index buffer read.
  // Queue: kRender.
  kIndexRead = 1 << 1,
  // Indicates that the access is a read via the vertex input bindings.
  // Queue: kRender.
  kVertexAttributeRead = 1 << 2,
  // Indicates that the access is a read via a uniform buffer or dynamic uniform
  // buffer descriptor.
  // Queue: kRender or kCompute.
  kUniformRead = 1 << 3,
  // Indicates that the access is a read via an input attachment descriptor.
  // Queue: kRender.
  kInputAttachmentRead = 1 << 4,
  // Indicates that the access is a read from a shader via any other descriptor
  // type.
  // Queue: kRender or kCompute.
  kShaderRead = 1 << 5,
  // Indicates that the access is a write or atomic from a shader via the same
  // descriptor types as in kShaderRead.
  // Queue: kRender or kCompute.
  kShaderWrite = 1 << 6,
  // Indicates that the access is a read via a color attachment.
  // Queue: kRender.
  kColorAttachmentRead = 1 << 7,
  // Indicates that the access is a write via a color or resolve attachment.
  // Queue: kRender.
  kColorAttachmentWrite = 1 << 8,
  // Indicates that the access is a read via a depth/stencil attachment.
  // Queue: kRender.
  kDepthStencilAttachmentRead = 1 << 9,
  // Indicates that the access is a write via a depth/stencil attachment.
  // Queue: kRender.
  kDepthStencilAttachmentWrite = 1 << 10,
  // Indicates that the access is a read from a transfer (copy, blit, resolve,
  // etc) operation.
  // Queue: any.
  kTransferRead = 1 << 11,
  // Indicates that the access is a write from a transfer (copy, blit, resolve,
  // etc) operation.
  // Queue: any.
  kTransferWrite = 1 << 12,
  // Indicates that the access is a read via the host.
  // Queue: any.
  kHostRead = 1 << 13,
  // Indicates that the access is a write via the host.
  // Queue: any.
  kHostWrite = 1 << 14,
  // Indicates that the access is a read via a non-specific unit attached to the
  // memory
  // Queue: any.
  kMemoryRead = 1 << 15,
  // Indicates that the access is a write via a non-specific unit attached to
  // the memory
  // Queue: any.
  kMemoryWrite = 1 << 16,
};
XRTL_BITMASK(AccessFlag);

class RenderPass : public RefObject<RenderPass> {
 public:
  // A sentinel that can be used in place of subpass indices to denote an
  // external data source. This may be used as a source to denote input
  // framebuffer data or a target to denote exported framebuffer data.
  static constexpr int kExternalSubpass = -1;

  // Defines how values are handled for attachments when loading.
  enum class LoadOp {
    // The previous contents of the image within the render area will be loaded
    // from memory and preserved.
    //
    // Uses either AccessFlag::kColorAttachmentRead or
    // kDepthStencilAttachmentRead.
    kLoad = 0,

    // The contents within the render area will be cleared to a uniform value
    // which is specified when a render pass instance is begun.
    //
    // Uses either AccessFlag::kColorAttachmentWrite or
    // kDepthStencilAttachmentWrite.
    kClear = 1,

    // The previous contents within the area need not be preserved; the contents
    // of the attachment will be undefined inside the render area.
    //
    // Uses either AccessFlag::kColorAttachmentWrite or
    // kDepthStencilAttachmentWrite.
    kDontCare = 2,
  };

  // Defines how values are handled for attachments when storing.
  enum class StoreOp {
    // The contents generated during the render pass and within the render area
    // are written to memory.
    //
    // Uses either AccessFlag::kColorAttachmentWrite or
    // kDepthStencilAttachmentWrite.
    kStore = 0,

    // The contents within the render area are not needed after rendering and
    // may be discarded; the contents of the attachment will be undefined inside
    // the render area.
    //
    // Uses either AccessFlag::kColorAttachmentWrite or
    // kDepthStencilAttachmentWrite.
    kDontCare = 1,
  };

  // Specifies an attachment for a render pass.
  //
  // Reference:
  // https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkAttachmentDescription.html
  struct AttachmentDescription {
    // Specifyies the format of the image that will be used for the
    // attachment.
    PixelFormat format = PixelFormats::kUndefined;
    // The number of samples of the image, if it is to be multisampled.
    SampleCount sample_count = SampleCount::k1;
    // Specifies how the contents of color and depth components of the
    // attachment are treated at the beginning of the subpass where it is first
    // used.
    LoadOp load_op = LoadOp::kDontCare;
    // Specifies how the contents of color and depth components of the
    // attachment are treated at the end of the subpass where it is last used.
    StoreOp store_op = StoreOp::kDontCare;
    // Specifies how the contents of stencil components of the attachment are
    // treated at the beginning of the subpass where it is first used.
    LoadOp stencil_load_op = LoadOp::kDontCare;
    // Specifies how the contents of stencil components of the attachment are
    // treated at the end of the last subpass where it is used.
    StoreOp stencil_store_op = StoreOp::kDontCare;
    // The layout the attachment image subresource will be in when a render pass
    // instance begins.
    Image::Layout initial_layout = Image::Layout::kUndefined;
    // The layout the attachment image subresource will be transitioned to when
    // a render pass instance ends. During a render pass instance an attachment
    // can use a different layout in each subpass, if desired.
    Image::Layout final_layout = Image::Layout::kGeneral;
  };

  // A reference to one of the attachments provided to the render pass.
  struct AttachmentReference {
    // Denotes that an attachment is not used and will not be written.
    static constexpr int kUnused = -1;
    // The index of the attachment of the render pass corresponding to the index
    // of the attachment in the render pass attachments array. If kUnused the
    // attachment is not used in the subpass and no writes will occur.
    int index = kUnused;
    // The layout the attachment uses during the subpass.
    Image::Layout layout = Image::Layout::kGeneral;

    AttachmentReference() = default;
    AttachmentReference(int index, Image::Layout layout)
        : index(index), layout(layout) {}
  };

  // Specifies a subpass description.
  //
  // Reference:
  // https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkSubpassDescription.html
  struct SubpassDescription {
    // An array listing which of the render pass's attachments can be read in
    // shaders during the subpass and what layout each attachment will be in
    // during the subpass.
    //
    // Each element of the array corresponds to an input attachment unit number
    // in the shader, i.e. if the shader declares an input variable
    // `layout(input_attachment_index=X, set=Y, binding=Z)` then it uses the
    // attachment provided in input_attachments[X]. Input attachments must also
    // be bound to the pipeline with a descriptor set with the input attachment
    // descriptor written in the location (set=Y, binding=Z).
    std::vector<AttachmentReference> input_attachments;

    // An array listing which of the render pass’s attachments will be used as
    // color attachments in the subpass and what layout each attachment will be
    // in during the subpass. Each element of the array corresponds to a
    // fragment shader output location, i.e. if the shader declared an output
    // variable `layout(location=X)` then it uses the attachment provided in
    // color_attachments[X].
    std::vector<AttachmentReference> color_attachments;

    // An array listing which of the render pass’s attachments are resolved to
    // at the end of the subpass and what layout each attachment will be in
    // during the multisample resolve operation. If this is not empty it must be
    // the same size as color_attachments and the indices between the two
    // correspond.
    std::vector<AttachmentReference> resolve_attachments;

    // Specifies which attachment will be used for depth/stencil data and the
    // layout it will be in during the subpass. Setting the attachment index to
    // kUnused indicates that no depth/stencil attachment will be used in the
    // subpass.
    AttachmentReference depth_stencil_attachment;

    // An array listing which of the render pass's attachments are not used by a
    // subpass but whose contents must be preserved throughout the subpass.
    std::vector<AttachmentReference> preserve_attachments;
  };

  // Specifies a subpass dependency.
  //
  // Reference:
  // https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkSubpassDependency.html
  struct SubpassDependency {
    // Index of the source subpass in the dependency or kExternalSubpass.
    int source_subpass = kExternalSubpass;
    // Index of the target subpass in the dependency or kExternalSubpass.
    int target_subpass = kExternalSubpass;
    PipelineStageFlag source_stage_mask;
    PipelineStageFlag target_stage_mask;
    AccessFlag source_access_mask;
    AccessFlag target_access_mask;
    PipelineDependencyFlag dependency_flags;
  };

  virtual ~RenderPass() = default;

  // A list of attachment descriptions.
  // Framebuffers must contain attachments corresponding to the indices of the
  // attachments described here. Each attachment must be format-compatible.
  const std::vector<AttachmentDescription>& attachments() const {
    return attachments_;
  }

  // A list of subpasses within the render pass.
  // All render passes need at least one subpass.
  const std::vector<SubpassDescription>& subpasses() const {
    return subpasses_;
  }

  // Declarations of dependencies between the subpasses within this render pass.
  const std::vector<SubpassDependency>& subpass_dependencies() const {
    return subpass_dependencies_;
  }

 protected:
  RenderPass(ArrayView<AttachmentDescription> attachments,
             ArrayView<SubpassDescription> subpasses,
             ArrayView<SubpassDependency> subpass_dependencies)
      : attachments_(attachments),
        subpasses_(subpasses),
        subpass_dependencies_(subpass_dependencies) {}

  std::vector<AttachmentDescription> attachments_;
  std::vector<SubpassDescription> subpasses_;
  std::vector<SubpassDependency> subpass_dependencies_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RENDER_PASS_H_
