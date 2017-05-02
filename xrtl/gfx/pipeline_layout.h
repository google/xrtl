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

#ifndef XRTL_GFX_PIPELINE_LAYOUT_H_
#define XRTL_GFX_PIPELINE_LAYOUT_H_

#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/render_pass.h"

namespace xrtl {
namespace gfx {

enum class BindingType {
  kSampler = 0,
  kCombinedImageSampler = 1,
  kSampledImage = 2,
  kStorageImage = 3,
  kUniformTexelBuffer = 4,
  kStorageTexelBuffer = 5,
  kUniformBuffer = 6,
  kStorageBuffer = 7,
  kUniformBufferDynamic = 8,
  kStorageBufferDynamic = 9,
  kInputAttachment = 10,
};

struct PipelineBinding {
  // Binding number of this entry and corresponds to a resource of the same
  // binding number in the shader stages.
  int binding = 0;
  // Specifies which type of resources are used for this binding.
  BindingType type = BindingType::kSampler;
  // The number of slots contained in the binding, accessed in a shader as
  // an array.
  int array_count = 0;
  // A bitmask specifying which pipeline shader stages can access a resource for
  // this binding.
  ShaderStageFlag stage_mask = ShaderStageFlag::kAll;
};

// Describes a range of push constant data within the pipeline.
struct PushConstantRange {
  // A set of stage flags describing the shader stages that will access a range
  // of push constants.
  ShaderStageFlag stage_mask = ShaderStageFlag::kNone;

  // Start offset and size, respectively, consumed by the range.
  // Both offset and size are in units of bytes and must be a multiple of 4.
  size_t offset = 0;
  size_t size = 0;
};

// Completely describes the layout of pipeline bindings.
// This is used to allow multiple pipelines to share the same descriptor sets.
//
// In Vulkan this encompasses both a PipelineLayout and set of
// DescriptorSetLayouts.
class PipelineLayout : public RefObject<PipelineLayout> {
 public:
  virtual ~PipelineLayout() = default;

  const std::vector<PipelineBinding>& bindings() const { return bindings_; }
  const std::vector<PushConstantRange>& push_constant_ranges() const {
    return push_constant_ranges_;
  }

 protected:
  PipelineLayout(ArrayView<PipelineBinding> bindings,
                 ArrayView<PushConstantRange> push_constant_ranges)
      : bindings_(bindings), push_constant_ranges_(push_constant_ranges) {}

  std::vector<PipelineBinding> bindings_;
  std::vector<PushConstantRange> push_constant_ranges_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_PIPELINE_LAYOUT_H_
