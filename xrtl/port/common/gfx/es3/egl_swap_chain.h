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

#ifndef XRTL_PORT_COMMON_GFX_ES3_EGL_SWAP_CHAIN_H_
#define XRTL_PORT_COMMON_GFX_ES3_EGL_SWAP_CHAIN_H_

#include <vector>

#include "xrtl/gfx/es3/es3_swap_chain.h"
#include "xrtl/port/common/gfx/es3/egl_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class EGLSwapChain : public ES3SwapChain {
 public:
  EGLSwapChain(ref_ptr<MemoryPool> memory_pool, ref_ptr<ui::Control> control,
               ref_ptr<EGLPlatformContext> platform_context,
               PresentMode present_mode, int image_count,
               ArrayView<PixelFormat> pixel_formats);
  ~EGLSwapChain() override;

  ResizeResult Resize(Size2D new_size) override;

  AcquireResult AcquireNextImage(std::chrono::milliseconds timeout_millis,
                                 ref_ptr<QueueFence> signal_queue_fence,
                                 ref_ptr<ImageView>* out_image_view) override;

  PresentResult PresentImage(
      ref_ptr<QueueFence> wait_queue_fence, ref_ptr<ImageView> image_view,
      std::chrono::milliseconds present_time_utc_millis) override;

 private:
  ref_ptr<MemoryPool> memory_pool_;
  ref_ptr<ui::Control> control_;
  ref_ptr<EGLPlatformContext> platform_context_;

  Image::CreateParams image_create_params_;
  std::vector<ref_ptr<ImageView>> image_views_;
  std::vector<GLuint> framebuffers_;
  int next_image_index_ = 0;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_GFX_ES3_EGL_SWAP_CHAIN_H_
