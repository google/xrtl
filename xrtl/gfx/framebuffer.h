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

#ifndef XRTL_GFX_FRAMEBUFFER_H_
#define XRTL_GFX_FRAMEBUFFER_H_

#include <utility>

#include "absl/container/inlined_vector.h"
#include "absl/types/span.h"
#include "xrtl/base/geometry.h"
#include "xrtl/gfx/image_view.h"
#include "xrtl/gfx/managed_object.h"
#include "xrtl/gfx/render_pass.h"

namespace xrtl {
namespace gfx {

// A render target framebuffer, composed of one or more attachments.
class Framebuffer : public ManagedObject {
 public:
  // Render pass this framebuffer is used with.
  ref_ptr<RenderPass> render_pass() const { return render_pass_; }
  // Dimensions of the framebuffer in pixels.
  Size3D size() const { return size_; }
  // Attachments for the framebuffer in the same order as specified in the
  // render pass.
  absl::Span<const ref_ptr<ImageView>> attachments() const {
    return attachments_;
  }

 protected:
  Framebuffer(ref_ptr<RenderPass> render_pass, Size3D size,
              absl::Span<const ref_ptr<ImageView>> attachments)
      : render_pass_(std::move(render_pass)),
        size_(size),
        attachments_(attachments.begin(), attachments.end()) {}

  ref_ptr<RenderPass> render_pass_;
  Size3D size_;
  absl::InlinedVector<ref_ptr<ImageView>, 4> attachments_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_FRAMEBUFFER_H_
