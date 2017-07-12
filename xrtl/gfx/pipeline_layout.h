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
#include "xrtl/gfx/resource_set_layout.h"

namespace xrtl {
namespace gfx {

// Completely describes the layout of pipeline bindings.
// This is used to allow multiple pipelines to share the same descriptor sets.
//
// PipelineLayout roughly maps to the following backend concepts:
// - Vulkan: pipeline layouts
class PipelineLayout : public RefObject<PipelineLayout> {
 public:
  // Describes a range of push constant data within the pipeline.
  struct PushConstantRange {
    // Start offset and size, respectively, consumed by the range.
    // Both offset and size are in units of bytes and must be a multiple of 4.
    size_t offset = 0;
    size_t size = 0;

    // A set of stage flags describing the shader stages that will access a
    // range of push constants.
    ShaderStageFlag stage_mask = ShaderStageFlag::kAll;

    PushConstantRange() = default;
    PushConstantRange(size_t offset, size_t size)
        : offset(offset), size(size) {}
    PushConstantRange(size_t offset, size_t size, ShaderStageFlag stage_mask)
        : offset(offset), size(size), stage_mask(stage_mask) {}
  };

  virtual ~PipelineLayout() = default;

  const std::vector<ref_ptr<ResourceSetLayout>>& resource_set_layouts() const {
    return resource_set_layouts_;
  }
  const std::vector<PushConstantRange>& push_constant_ranges() const {
    return push_constant_ranges_;
  }

 protected:
  PipelineLayout(
      ArrayView<ref_ptr<ResourceSetLayout>> resource_set_layouts,
      ArrayView<PipelineLayout::PushConstantRange> push_constant_ranges)
      : resource_set_layouts_(resource_set_layouts),
        push_constant_ranges_(push_constant_ranges) {}

  std::vector<ref_ptr<ResourceSetLayout>> resource_set_layouts_;
  std::vector<PipelineLayout::PushConstantRange> push_constant_ranges_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_PIPELINE_LAYOUT_H_
