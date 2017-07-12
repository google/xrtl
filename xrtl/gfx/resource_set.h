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

#ifndef XRTL_GFX_RESOURCE_SET_H_
#define XRTL_GFX_RESOURCE_SET_H_

#include <memory>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/buffer.h"
#include "xrtl/gfx/image.h"
#include "xrtl/gfx/image_view.h"
#include "xrtl/gfx/resource_set_layout.h"
#include "xrtl/gfx/sampler.h"

namespace xrtl {
namespace gfx {

// A set of bindings for a particular PipelineLayout.
// Bindings can be used across multiple Pipelines that share the same layout.
//
// When a particular set of resource bindings is immutable it's recommended to
// retain that ResourceSet instance and reuse it across many command buffers.
// If the set of resource bindings may change during execution (whether within
// the same command buffer or across command buffers) it's recommended to split
// the immutable from the mutable and create one-shot ResourceSet instances for
// the mutable portions.
//
// ResourceSet roughly maps to the following backend concepts:
// - D3D12: descriptor tables
// - Metal: argument buffers
// - Vulkan: descriptor sets
class ResourceSet : public RefObject<ResourceSet> {
 public:
  // Describes a single binding slot within the resource set.
  struct BindingValue {
    // Array element children.
    std::vector<BindingValue> elements;

    // Buffer bound to the slot, or nullptr for none.
    ref_ptr<Buffer> buffer;
    // The offset in bytes from the start of buffer. Access to buffer memory via
    // this descriptor uses addressing that is relative to this starting offset.
    size_t buffer_offset = 0;
    // The size in bytes that is used for this descriptor update, or -1 to use
    // the range from offset to the end of the buffer.
    size_t buffer_length = -1;

    // Image view.
    ref_ptr<ImageView> image_view;
    // Layout the image is in when bound.
    Image::Layout image_layout = Image::Layout::kGeneral;

    // Sampler used for the image.
    ref_ptr<Sampler> sampler;

    BindingValue() = default;

    // Binding slot used for arrays of bindings.
    BindingValue(ArrayView<BindingValue> elements)  // NOLINT
        : elements(elements) {}

    // Binding slot used for kUniformBuffer and kStorageBuffer.
    BindingValue(ref_ptr<Buffer> buffer)  // NOLINT
        : buffer(std::move(buffer)) {}
    BindingValue(ref_ptr<Buffer> buffer, size_t offset, size_t length)
        : buffer(std::move(buffer)),
          buffer_offset(offset),
          buffer_length(length) {}

    // Binding slot used for kSampledImage, kStorageImage, and kInputAttachment.
    BindingValue(ref_ptr<ImageView> image_view, Image::Layout image_layout)
        : image_view(std::move(image_view)), image_layout(image_layout) {}

    // Binding slot used for kSampler.
    BindingValue(ref_ptr<Sampler> sampler)  // NOLINT
        : sampler(std::move(sampler)) {}

    // Binding slot used for kCombinedImageSampler.
    BindingValue(ref_ptr<ImageView> image_view, Image::Layout image_layout,
                 ref_ptr<Sampler> sampler)
        : image_view(std::move(image_view)),
          image_layout(image_layout),
          sampler(std::move(sampler)) {}
  };

  virtual ~ResourceSet() = default;

  // Layout the resource set uses.
  ref_ptr<ResourceSetLayout> layout() const { return layout_; }

  // All bindings for the resource set.
  const std::vector<BindingValue>& binding_values() const {
    return binding_values_;
  }

 protected:
  explicit ResourceSet(ref_ptr<ResourceSetLayout> resource_set_layout,
                       ArrayView<BindingValue> binding_values)
      : layout_(std::move(resource_set_layout)),
        binding_values_(binding_values) {}

  ref_ptr<ResourceSetLayout> layout_;
  std::vector<BindingValue> binding_values_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_RESOURCE_SET_H_
