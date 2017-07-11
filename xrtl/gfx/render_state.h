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

#ifndef XRTL_GFX_RENDER_STATE_H_
#define XRTL_GFX_RENDER_STATE_H_

#include <cstdint>

#include "xrtl/base/fixed_vector.h"
#include "xrtl/gfx/pixel_format.h"
#include "xrtl/gfx/vertex_format.h"

namespace xrtl {
namespace gfx {

// Maximum number of vertex bindings and attributes.
// There may be a larger number supported by the device but this is all the
// space we reserve for now.
constexpr int kMaxVertexInputs = 16;

// Maximum number of color attachments.
// There may be a larger number supported by the device but this is all the
// space we reserve for now.
constexpr int kMaxColorAttachments = 8;

enum class PrimitiveTopology {
  kPointList = 0,
  kLineList = 1,
  kLineStrip = 2,
  kTriangleList = 3,
  kTriangleStrip = 4,
  kTriangleFan = 5,
  kLineListWithAdjacency = 6,
  kLineStripWithAdjacency = 7,
  kTriangleListWithAdjacency = 8,
  kTriangleStripWithAdjacency = 9,
  kPatchList = 10,
};
constexpr int kPrimitiveTopologyBits = 4;

enum class CullMode {
  kNone = 0,
  kFront = 1,
  kBack = 2,
  kFrontAndBack = 3,
};
constexpr int kCullModeBits = 3;

enum class FrontFace {
  kCounterClockwise = 0,
  kClockwise = 1,
};
constexpr int kFrontFaceBits = 1;

// Sample count used for multisampling.
enum class SampleCount : uint32_t {
  k1 = 1,
  k2 = 2,
  k4 = 4,
  k8 = 8,
  k16 = 16,
  k32 = 32,
  k64 = 64,
};
XRTL_BITMASK(SampleCount);
constexpr int kSampleCountBits = 7;

// Reference:
// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBlendFactor.html
enum class BlendFactor {
  // Color: (0,0,0)
  // Alpha: 0
  kZero = 0,
  // Color: (1,1,1)
  // Alpha: 1
  kOne = 1,
  // Color: (Rs0,Gs0,Bs0)
  // Alpha: As0
  kSrcColor = 2,
  // Color: (1-Rs0,1-Gs0,1-Bs0)
  // Alpha: 1-As0
  kOneMinusSrcColor = 3,
  // Color: (Rd,Gd,Bd)
  // Alpha: Ad
  kDstColor = 4,
  // Color: (1-Rd,1-Gd,1-Bd)
  // Alpha: 1-Ad
  kOneMinusDstColor = 5,
  // Color: (As0,As0,As0)
  // Alpha: As0
  kSrcAlpha = 6,
  // Color: (1-As0,1-As0,1-As0)
  // Alpha: 1-As0
  kOneMinusSrcAlpha = 7,
  // Color: (Ad,Ad,Ad)
  // Alpha: Ad
  kDstAlpha = 8,
  // Color: (1-Ad,1-Ad,1-Ad)
  // Alpha: 1-Ad
  kOneMinusDstAlpha = 9,
  // Color: (Rc,Gc,Bc)
  // Alpha: Ac
  kConstantColor = 10,
  // Color: (1-Rc,1-Gc,1-Bc)
  // Alpha: 1-Ac
  kOneMinusConstantColor = 11,
  // Color: (Ac,Ac,Ac)
  // Alpha: Ac
  kConstantAlpha = 12,
  // Color: (1-Ac,1-Ac,1-Ac)
  // Alpha: 1-Ac
  kOneMinusConstantAlpha = 13,
  // Color: (f,f,f); f = min(As0,1-Ad)
  // Alpha: 1
  kSrcAlphaSaturate = 14,
};
constexpr int kBlendFactorBits = 4;

// Reference:
// https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkBlendOp.html
enum class BlendOp {
  // R = Rs0 × Sr + Rd × Dr
  // G = Gs0 × Sg + Gd × Dg
  // B = Bs0 × Sb + Bd × Db
  // A = As0 × Sa + Ad × Da
  kAdd = 0,

  // R = Rs0 × Sr - Rd × Dr
  // G = Gs0 × Sg - Gd × Dg
  // B = Bs0 × Sb - Bd × Db
  // A = As0 × Sa - Ad × Da
  kSubtract = 1,

  // R = Rd × Dr - Rs0 × Sr
  // G = Gd × Dg - Gs0 × Sg
  // B = Bd × Db - Bs0 × Sb
  // A = Ad × Da - As0 × Sa
  kReverseSubtract = 2,

  // R = min(Rs0,Rd)
  // G = min(Gs0,Gd)
  // B = min(Bs0,Bd)
  // A = min(As0,Ad)
  kMin = 3,

  // R = max(Rs0,Rd)
  // G = max(Gs0,Gd)
  // B = max(Bs0,Bd)
  // A = max(As0,Ad)
  kMax = 4,
};
constexpr int kBlendOpBits = 3;

enum class ColorComponentMask : uint32_t {
  kR = 0x1,
  kG = 0x2,
  kB = 0x4,
  kA = 0x8,
  kRGB = kR | kG | kB,
  kRGBA = kR | kG | kB | kA,
};
XRTL_BITMASK(ColorComponentMask);
constexpr int kColorComponentMaskBits = 4;

enum class VertexInputRate {
  // Indicates that vertex attribute addressing is a function of the vertex
  // index.
  kVertex = 0,
  // Indicates that vertex attribute addressing is a function of the instance
  // index.
  kInstance = 1,
};

struct VertexInputBinding {
  // The binding number that this structure describes.
  int binding = 0;
  // Distance in bytes between two consecutive elements within the buffer.
  size_t stride = 0;
  // Specifies whether vertex attribute addressing is a function of the vertex
  // index or of the instance index.
  VertexInputRate input_rate = VertexInputRate::kVertex;

  VertexInputBinding() = default;
  VertexInputBinding(int binding, size_t stride)
      : binding(binding), stride(stride) {}
  VertexInputBinding(int binding, size_t stride, VertexInputRate input_rate)
      : binding(binding), stride(stride), input_rate(input_rate) {}
};

struct VertexInputAttribute {
  // The shader binding location number for this attribute.
  int location = 0;
  // The binding number which this attribute takes its data from.
  int binding = 0;
  // A byte offset of this attribute relative to the start of an element in the
  // vertex input binding.
  size_t offset = 0;
  // The size and type of the vertex attribute data.
  VertexFormat format = VertexFormats::kUndefined;

  VertexInputAttribute() = default;
  VertexInputAttribute(int location, int binding, size_t offset,
                       VertexFormat format)
      : location(location), binding(binding), offset(offset), format(format) {}
};

class RenderState {
 public:
  class VertexInputState {
   public:
    VertexInputState() = default;

    FixedVector<VertexInputBinding, kMaxVertexInputs> vertex_bindings;
    FixedVector<VertexInputAttribute, kMaxVertexInputs> vertex_attributes;
  };
  VertexInputState vertex_input_state;

  class InputAssemblyState {
   public:
    InputAssemblyState() {
      primitive_topology_ = PrimitiveTopology::kTriangleList;
      primitive_restart_enabled_ = false;
    }

    // Defines the primitive topology.
    PrimitiveTopology primitive_topology() const { return primitive_topology_; }
    void set_primitive_topology(PrimitiveTopology value) {
      primitive_topology_ = value;
    }

    // Whether a special vertex index value is treated as restarting the
    // assembly of primitives when performing an indexed draw. For 16-bit
    // index buffers the value is 0xFFFF and for 32-bit index buffers the value
    // is 0xFFFFFFFF.
    bool is_primitive_restart_enabled() const {
      return primitive_restart_enabled_;
    }
    void set_primitive_restart_enabled(bool value) {
      primitive_restart_enabled_ = value;
    }

   private:
    PrimitiveTopology primitive_topology_ : kPrimitiveTopologyBits;
    bool primitive_restart_enabled_ : 1;
  };
  InputAssemblyState input_assembly_state;

  class TessellationState {
   public:
    TessellationState() { patch_control_points_ = 1; }

    // Number of control points per patch.
    int patch_control_points() const { return patch_control_points_; }
    void set_patch_control_points(int value) { patch_control_points_ = value; }

   private:
    int patch_control_points_;
  };
  TessellationState tessellation_state;

  class ViewportState {
   public:
    ViewportState() { count_ = 1; }

    // Total number of viewports enabled during this pipeline.
    int count() const { return count_; }
    void set_count(int value) { count_ = value; }

   private:
    int count_;
  };
  ViewportState viewport_state;

  class RasterizationState {
   public:
    RasterizationState() {
      rasterizer_discard_enabled_ = false;
      cull_mode_ = CullMode::kNone;
      front_face_ = FrontFace::kCounterClockwise;
      depth_bias_enabled_ = false;
    }

    // Controls whether primitives are discarded immediately before the
    // rasterization stage.
    bool is_rasterizer_discard_enabled() const {
      return rasterizer_discard_enabled_;
    }
    void set_rasterizer_discard_enabled(bool value) {
      rasterizer_discard_enabled_ = value;
    }

    // The triangle facing direction used for primitive culling.
    CullMode cull_mode() const { return cull_mode_; }
    void set_cull_mode(CullMode value) { cull_mode_ = value; }

    // The front-facing triangle orientation to be used for culling.
    FrontFace front_face() const { return front_face_; }
    void set_front_face(FrontFace value) { front_face_ = value; }

    // Controls whether to bias fragment depth values.
    bool is_depth_bias_enabled() const { return depth_bias_enabled_; }
    void set_depth_bias_enabled(bool value) { depth_bias_enabled_ = value; }

   private:
    bool rasterizer_discard_enabled_ : 1;
    CullMode cull_mode_ : kCullModeBits;
    FrontFace front_face_ : kFrontFaceBits;
    bool depth_bias_enabled_ : 1;
  };
  RasterizationState rasterization_state;

  class MultisampleState {
   public:
    MultisampleState() {
      rasterization_samples_ = SampleCount::k1;
      alpha_to_coverage_enabled_ = false;
      alpha_to_one_enabled_ = false;
      sample_shading_enabled_ = false;
      min_sample_shading_ = 0.0f;
    }

    // Specifies the number of samples per pixel used in rasterization.
    SampleCount rasterization_samples() const { return rasterization_samples_; }
    void set_rasterization_samples(SampleCount value) {
      rasterization_samples_ = value;
    }

    // Controls whether a temporary coverage value is generated based on the
    // alpha component of the fragment’s first color output.
    bool is_alpha_to_coverage_enabled() const {
      return alpha_to_coverage_enabled_;
    }
    void set_alpha_to_coverage_enabled(bool value) {
      alpha_to_coverage_enabled_ = value;
    }

    // Controls whether the alpha component of the fragment’s first color output
    // is replaced with one.
    bool is_alpha_to_one_enabled() const { return alpha_to_one_enabled_; }
    void set_alpha_to_one_enabled(bool value) { alpha_to_one_enabled_ = value; }

    // True if fragment shading executes per-sample, otherwise per-fragment.
    bool is_sample_shading_enabled() const { return sample_shading_enabled_; }
    void set_sample_shading_enabled(bool value) {
      sample_shading_enabled_ = value;
    }

    // The minimum fraction of sample shading.
    float min_sample_shading() const { return min_sample_shading_; }
    void set_min_sample_shading(float value) { min_sample_shading_ = value; }

    // TODO(benvanik): sample mask array, each element uint32_t, count is
    //                 rasterization_samples.
    // const VkSampleMask* pSampleMask;

   private:
    SampleCount rasterization_samples_ : kSampleCountBits;
    bool alpha_to_coverage_enabled_ : 1;
    bool alpha_to_one_enabled_ : 1;
    bool sample_shading_enabled_ : 1;
    float min_sample_shading_;
  };
  MultisampleState multisample_state;

  // Determines the stencil comparison function.
  // R is the masked reference value and S is the masked stored stencil value.
  enum class CompareOp {
    // The test never passes.
    kNever = 0,
    // The test passes when R < S.
    kLess = 1,
    // The test passes when R = S.
    kEqual = 2,
    // The test passes when R ≤ S.
    kLessOrEqual = 3,
    // The test passes when R > S.
    kGreater = 4,
    // The test passes when R ≠ S.
    kNotEqual = 5,
    // The test passes when R ≥ S.
    kGreaterOrEqual = 6,
    // The test always passes.
    kAlways = 7,
  };
  static constexpr int kCompareOpBits = 3;

  // Indicates what happens to the stored stencil value if this or certain
  // subsequent tests fail or pass.
  enum class StencilOp {
    // Keeps the current value.
    kKeep = 0,
    // Sets the value to 0.
    kZero = 1,
    // Sets the value to reference.
    kReplace = 2,
    // Increments the current value and clamps to the maximum representable
    // unsigned value.
    kIncrementAndClamp = 3,
    // Decrements the current value and clamps to 0.
    kDecrementAndClamp = 4,
    // Bitwise-inverts the current value.
    kInvert = 5,
    // Increments the current value and wraps to 0 when the maximum value would
    // have been exceeded.
    kIncrementAndWrap = 6,
    // Decrements the current value and wraps to the maximum possible value when
    // the value would go below 0.
    kDecrementAndWrap = 7,
  };
  static constexpr int kStencilOpBits = 3;

  class StencilOpState {
   public:
    StencilOpState() {
      fail_op_ = StencilOp::kKeep;
      pass_op_ = StencilOp::kKeep;
      depth_fail_op_ = StencilOp::kKeep;
      compare_op_ = CompareOp::kAlways;
    }

    StencilOp fail_op() const { return fail_op_; }
    void set_fail_op(StencilOp value) { fail_op_ = value; }

    StencilOp pass_op() const { return pass_op_; }
    void set_pass_op(StencilOp value) { pass_op_ = value; }

    StencilOp depth_fail_op() const { return depth_fail_op_; }
    void set_depth_fail_op(StencilOp value) { depth_fail_op_ = value; }

    CompareOp compare_op() const { return compare_op_; }
    void set_compare_op(CompareOp value) { compare_op_ = value; }

   private:
    StencilOp fail_op_ : kStencilOpBits;
    StencilOp pass_op_ : kStencilOpBits;
    StencilOp depth_fail_op_ : kStencilOpBits;
    CompareOp compare_op_ : kCompareOpBits;
  };

  class DepthStencilState {
   public:
    DepthStencilState() {
      depth_test_enabled_ = false;
      depth_write_enabled_ = false;
      depth_compare_op_ = CompareOp::kLess;
      depth_bounds_test_enabled_ = false;
      stencil_test_enabled_ = false;
    }

    bool is_depth_test_enabled() const { return depth_test_enabled_; }
    void set_depth_test_enabled(bool value) { depth_test_enabled_ = value; }

    bool is_depth_write_enabled() const { return depth_write_enabled_; }
    void set_depth_write_enabled(bool value) { depth_write_enabled_ = value; }

    CompareOp depth_compare_op() const { return depth_compare_op_; }
    void set_depth_compare_op(CompareOp value) { depth_compare_op_ = value; }

    bool is_depth_bounds_test_enabled() const {
      return depth_bounds_test_enabled_;
    }
    void set_depth_bounds_test_enabled(bool value) {
      depth_bounds_test_enabled_ = value;
    }

    bool is_stencil_test_enabled() const { return stencil_test_enabled_; }
    void set_stencil_test_enabled(bool value) { stencil_test_enabled_ = value; }

    const StencilOpState& stencil_front_state() const {
      return stencil_front_state_;
    }
    void set_stencil_front_state(StencilOpState value) {
      stencil_front_state_ = value;
    }
    const StencilOpState& stencil_back_state() const {
      return stencil_back_state_;
    }
    void set_stencil_back_state(StencilOpState value) {
      stencil_back_state_ = value;
    }

   private:
    bool depth_test_enabled_ : 1;
    bool depth_write_enabled_ : 1;
    CompareOp depth_compare_op_ : kCompareOpBits;
    bool depth_bounds_test_enabled_ : 1;
    bool stencil_test_enabled_ : 1;
    StencilOpState stencil_front_state_;
    StencilOpState stencil_back_state_;
  };
  DepthStencilState depth_stencil_state;

  class ColorBlendAttachmentState {
   public:
    ColorBlendAttachmentState() {
      blend_enabled_ = false;
      src_color_blend_factor_ = BlendFactor::kOne;
      dst_color_blend_factor_ = BlendFactor::kZero;
      color_blend_op_ = BlendOp::kAdd;
      src_alpha_blend_factor_ = BlendFactor::kOne;
      dst_alpha_blend_factor_ = BlendFactor::kZero;
      alpha_blend_op_ = BlendOp::kAdd;
      color_write_mask_ = ColorComponentMask::kRGBA;
    }

    // Controls whether blending is enabled for the corresponding color
    // attachment. If blending is not enabled the source fragment’s color for
    // that attachment is passed through unmodified.
    bool is_blend_enabled() const { return blend_enabled_; }
    void set_blend_enabled(bool value) { blend_enabled_ = value; }

    // Selects which blend factor is used to determine the source factors
    // (Sr,Sg,Sb).
    BlendFactor src_color_blend_factor() const {
      return src_color_blend_factor_;
    }
    void set_src_color_blend_factor(BlendFactor value) {
      src_color_blend_factor_ = value;
    }

    // Selects which blend factor is used to determine the destination factors
    // (Dr,Dg,Db).
    BlendFactor dst_color_blend_factor() const {
      return dst_color_blend_factor_;
    }
    void set_dst_color_blend_factor(BlendFactor value) {
      dst_color_blend_factor_ = value;
    }

    // Selects which blend operation is used to calculate the RGB values to
    // write to the color attachment.
    BlendOp color_blend_op() const { return color_blend_op_; }
    void set_color_blend_op(BlendOp value) { color_blend_op_ = value; }

    // Selects which blend factor is used to determine the source factor Sa.
    BlendFactor src_alpha_blend_factor() const {
      return src_alpha_blend_factor_;
    }
    void set_src_alpha_blend_factor(BlendFactor value) {
      src_alpha_blend_factor_ = value;
    }

    // Selects which blend factor is used to determine the destination factor
    // Da.
    BlendFactor dst_alpha_blend_factor() const {
      return dst_alpha_blend_factor_;
    }
    void set_dst_alpha_blend_factor(BlendFactor value) {
      dst_alpha_blend_factor_ = value;
    }

    // Selects which blend operation is use to calculate the alpha values to
    // write to the color attachment.
    BlendOp alpha_blend_op() const { return alpha_blend_op_; }
    void set_alpha_blend_op(BlendOp value) { alpha_blend_op_ = value; }

    // Sets the src blend factor used by both color and alpha.
    void set_src_blend_factor(BlendFactor value) {
      set_src_color_blend_factor(value);
      set_src_alpha_blend_factor(value);
    }

    // Sets the dst blend factor used by both color and alpha.
    void set_dst_blend_factor(BlendFactor value) {
      set_dst_color_blend_factor(value);
      set_dst_alpha_blend_factor(value);
    }

    // Sets the blend op used by both color and alpha.
    void set_blend_op(BlendOp value) {
      set_color_blend_op(value);
      set_alpha_blend_op(value);
    }

    // A bitmask selecting which of the R, G, B, and/or A components are enabled
    // for writing.
    ColorComponentMask color_write_mask() const { return color_write_mask_; }
    void set_color_write_mask(ColorComponentMask value) {
      color_write_mask_ = value;
    }

   private:
    bool blend_enabled_ : 1;
    BlendFactor src_color_blend_factor_ : kBlendFactorBits;
    BlendFactor dst_color_blend_factor_ : kBlendFactorBits;
    BlendOp color_blend_op_ : kBlendOpBits;
    BlendFactor src_alpha_blend_factor_ : kBlendFactorBits;
    BlendFactor dst_alpha_blend_factor_ : kBlendFactorBits;
    BlendOp alpha_blend_op_ : kBlendOpBits;
    ColorComponentMask color_write_mask_ : kColorComponentMaskBits;
  };

  class ColorBlendState {
   public:
    ColorBlendState() {}

    // An array of states, one for each subpass attachment.
    // The indices match with the subpass color attachments.
    // If no attachment settings are specified all attachments will have the
    // default blend mode.
    //
    // Compatibility note:
    // - OpenGL ES: all attachments must have the same state.
    FixedVector<ColorBlendAttachmentState, kMaxColorAttachments> attachments;

   private:
  };
  ColorBlendState color_blend_state;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RENDER_STATE_H_
