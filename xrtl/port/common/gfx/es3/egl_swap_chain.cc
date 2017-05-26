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

#include "xrtl/port/common/gfx/es3/egl_swap_chain.h"

#include <utility>

#include "xrtl/base/threading/thread.h"
#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_image.h"
#include "xrtl/gfx/es3/es3_queue_fence.h"
#include "xrtl/port/common/gfx/es3/egl_strings.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ref_ptr<ES3SwapChain> ES3SwapChain::Create(
    ref_ptr<ES3PlatformContext> shared_platform_context,
    ref_ptr<MemoryPool> memory_pool, ref_ptr<ui::Control> control,
    PresentMode present_mode, int image_count,
    ArrayView<PixelFormat> pixel_formats) {
  WTF_SCOPE0("ES3SwapChain#Create");

  // Create the context targeting the native window.
  // This is the only way in (base) EGL to get a hardware framebuffer.
  auto platform_context = ES3PlatformContext::Create(
      reinterpret_cast<void*>(control->platform_display_handle()),
      reinterpret_cast<void*>(control->platform_handle()),
      shared_platform_context);
  if (!platform_context) {
    LOG(ERROR) << "Unable to initialize the swap chain EGL platform context";
    return nullptr;
  }

  return make_ref<EGLSwapChain>(memory_pool, control,
                                platform_context.As<EGLPlatformContext>(),
                                present_mode, image_count, pixel_formats);
}

EGLSwapChain::EGLSwapChain(ref_ptr<MemoryPool> memory_pool,
                           ref_ptr<ui::Control> control,
                           ref_ptr<EGLPlatformContext> platform_context,
                           PresentMode present_mode, int image_count,
                           ArrayView<PixelFormat> pixel_formats)
    : ES3SwapChain(present_mode, image_count, pixel_formats),
      memory_pool_(memory_pool),
      control_(std::move(control)),
      platform_context_(platform_context) {
  WTF_SCOPE0("EGLSwapChain#ctor");
  ES3PlatformContext::ThreadLock context_lock(platform_context_);

  // Query the initial surface size.
  size_ = platform_context_->QuerySize();

  image_create_params_.format = pixel_formats[0];
  image_create_params_.size = Size3D(size_);
  image_create_params_.usage_mask =
      Image::Usage::kTransferSource | Image::Usage::kSampled |
      Image::Usage::kColorAttachment | Image::Usage::kInputAttachment;

  // Allocate framebuffers we'll use for copying.
  framebuffers_.resize(image_count);
  glGenFramebuffers(framebuffers_.size(), framebuffers_.data());

  // Allocate initial images.
  image_views_.resize(image_count);
  Resize(size_);
}

EGLSwapChain::~EGLSwapChain() {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));
  glDeleteFramebuffers(framebuffers_.size(), framebuffers_.data());
}

SwapChain::ResizeResult EGLSwapChain::Resize(Size2D new_size) {
  WTF_SCOPE0("EGLSwapChain#Resize");
  ES3PlatformContext::ThreadLock context_lock(platform_context_);

  // Recreate the underlying surface.
  auto recreate_surface_result = platform_context_->RecreateSurface(new_size);
  switch (recreate_surface_result) {
    case EGLPlatformContext::RecreateSurfaceResult::kSuccess:
      // OK.
      break;
    case EGLPlatformContext::RecreateSurfaceResult::kInvalidTarget:
      LOG(ERROR) << "Failed to recreate swap chain surface; invalid target";
      return ResizeResult::kDeviceLost;
    case EGLPlatformContext::RecreateSurfaceResult::kOutOfMemory:
      LOG(ERROR) << "Failed to recreate swap chain surface; out of memory";
      return ResizeResult::kOutOfMemory;
    case EGLPlatformContext::RecreateSurfaceResult::kDeviceLost:
      LOG(ERROR) << "Failed to recreate swap chain surface; device lost";
      return ResizeResult::kDeviceLost;
  }

  // Query the new size, as it may be different than requested.
  size_ = platform_context_->QuerySize();
  image_create_params_.size = Size3D(size_);

  // Resize all images by recreating them.
  for (int i = 0; i < image_views_.size(); ++i) {
    image_views_[i].reset();
  }
  memory_pool_->Reclaim();
  for (int i = 0; i < image_views_.size(); ++i) {
    // Allocate image.
    ref_ptr<Image> image;
    auto result = memory_pool_->AllocateImage(image_create_params_, &image);
    DCHECK(result == MemoryPool::AllocationResult::kSuccess);
    if (!image) {
      return ResizeResult::kOutOfMemory;
    }

    // Get a view for the target format.
    image_views_[i] =
        image->CreateView(Image::Type::k2D, image_create_params_.format);

    // Bind texture to framebuffer.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers_[i]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, image.As<ES3Image>()->texture_id(),
                           0);
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  return ResizeResult::kSuccess;
}

SwapChain::AcquireResult EGLSwapChain::AcquireNextImage(
    std::chrono::milliseconds timeout_millis,
    ref_ptr<QueueFence> signal_queue_fence,
    ref_ptr<ImageView>* out_image_view) {
  WTF_SCOPE0("EGLSwapChain#AcquireNextImage");
  ES3PlatformContext::ThreadLock context_lock(platform_context_);

  // NOTE: we always signal the fence right away, as we don't support overlap.
  signal_queue_fence.As<ES3QueueFence>()->event()->Set();

  int image_index = next_image_index_;
  next_image_index_ = (next_image_index_ + 1) % image_views_.size();
  *out_image_view = image_views_[image_index];

  return AcquireResult::kSuccess;
}

SwapChain::PresentResult EGLSwapChain::PresentImage(
    ref_ptr<QueueFence> wait_queue_fence, ref_ptr<ImageView> image_view,
    std::chrono::milliseconds present_time_utc_millis) {
  WTF_SCOPE0("EGLSwapChain#PresentImage");
  ES3PlatformContext::ThreadLock context_lock(platform_context_);

  // Wait for the queue fence to trigger.
  Thread::Wait(wait_queue_fence.As<ES3QueueFence>()->event());

  // Query current size from the context.
  Size2D surface_size = control_->size();
  bool resize_required = surface_size != size_;

  // Map image view back to a GL framebuffer.
  GLuint framebuffer_id = 0;
  GLuint texture_id = 0;
  for (int i = 0; i < image_views_.size(); ++i) {
    if (image_views_[i] == image_view) {
      framebuffer_id = framebuffers_[i];
      texture_id = image_view->image().As<ES3Image>()->texture_id();
      break;
    }
  }
  DCHECK_NE(framebuffer_id, 0);
  DCHECK_NE(texture_id, 0);

  // TODO(benvanik): multisample resolve, scaling, etc.

  // Bind our source (read) framebuffer, which is the image the content was
  // rendered into.
  // NOTE: because we use the texture in other framebuffers we *must* reattach
  //       here; GL will implicitly drop attachments from all other framebuffers
  //       when a texture is attached to another.
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_id);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, texture_id, 0);
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  // Bind the native swap surface framebuffer.
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  GLenum draw_buffer = GL_BACK;
  glDrawBuffers(1, &draw_buffer);

  glViewport(0, 0, surface_size.width, surface_size.height);

  glBlitFramebuffer(0, 0, size_.width, size_.height, 0, 0, surface_size.width,
                    surface_size.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

  platform_context_->SwapBuffers(present_time_utc_millis);

  return resize_required ? PresentResult::kResizeRequired
                         : PresentResult::kSuccess;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
