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

#ifndef XRTL_GFX_RESOURCE_SET_LAYOUT_H_
#define XRTL_GFX_RESOURCE_SET_LAYOUT_H_

#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/render_pass.h"

namespace xrtl {
namespace gfx {

// Defines the binding slots used within a ResourceSet.
// ResourceSets are considered compatible if they share the same layout.
//
// ResourceSetLayout roughly maps to the following backend concepts:
// - D3D12: descriptor tables
// - Metal: argument buffers
// - Vulkan: descriptor set layouts
class ResourceSetLayout : public RefObject<ResourceSetLayout> {
 public:
  struct BindingSlot {
    enum class Type {
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

    // Binding number of this entry and corresponds to a resource of the same
    // binding number in the shader stages.
    int binding = 0;
    // Specifies which type of resources are used for this binding.
    Type type = Type::kCombinedImageSampler;
    // The number of slots contained in the binding, accessed in a shader as
    // an array.
    int array_count = 1;
    // A bitmask specifying which pipeline shader stages can access a resource
    // for this binding.
    ShaderStageFlag stage_mask = ShaderStageFlag::kAll;

    BindingSlot() = default;
    BindingSlot(int binding, Type type) : binding(binding), type(type) {}
    BindingSlot(int binding, Type type, int array_count)
        : binding(binding), type(type), array_count(array_count) {}
    BindingSlot(int binding, Type type, int array_count,
                ShaderStageFlag stage_mask)
        : binding(binding),
          type(type),
          array_count(array_count),
          stage_mask(stage_mask) {}
  };

  virtual ~ResourceSetLayout() = default;

  const std::vector<BindingSlot>& binding_slots() const {
    return binding_slots_;
  }

 protected:
  explicit ResourceSetLayout(ArrayView<BindingSlot> binding_slots)
      : binding_slots_(binding_slots) {}

  std::vector<BindingSlot> binding_slots_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RESOURCE_SET_LAYOUT_H_
