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

#include "xrtl/gfx/es3/es3_swap_chain.h"

#include <utility>

#include "xrtl/base/system_clock.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_image.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// TODO(benvanik): remove the need for this when we have multiple impls.
ref_ptr<ES3SwapChain> ES3SwapChain::Create(
    ref_ptr<ES3PlatformContext> shared_platform_context,
    ES3Queue* present_queue, ref_ptr<MemoryHeap> memory_heap,
    ref_ptr<ui::Control> control, PresentMode present_mode, int image_count,
    ArrayView<PixelFormat> pixel_formats) {
  WTF_SCOPE0("ES3SwapChain#Create");

  // Create the context targeting the native window.
  // This is the only way in (base) WGL to get a hardware framebuffer.
  auto platform_context = ES3PlatformContext::Create(
      reinterpret_cast<void*>(control->platform_display_handle()),
      reinterpret_cast<void*>(control->platform_handle()),
      std::move(shared_platform_context));
  if (!platform_context) {
    LOG(ERROR) << "Unable to initialize the swap chain WGL platform context";
    return nullptr;
  }

  auto swap_chain = make_ref<ES3PlatformSwapChain>(
      present_queue, memory_heap, control, platform_context, present_mode,
      image_count, pixel_formats);
  if (!swap_chain->Initialize()) {
    return nullptr;
  }
  return swap_chain;
}

ES3PlatformSwapChain::ES3PlatformSwapChain(
    ES3Queue* present_queue, ref_ptr<MemoryHeap> memory_heap,
    ref_ptr<ui::Control> control, ref_ptr<ES3PlatformContext> platform_context,
    PresentMode present_mode, int image_count,
    ArrayView<PixelFormat> pixel_formats)
    : ES3SwapChain(present_mode, image_count, pixel_formats),
      present_queue_(present_queue),
      memory_heap_(std::move(memory_heap)),
      control_(std::move(control)),
      platform_context_(std::move(platform_context)) {}

ES3PlatformSwapChain::~ES3PlatformSwapChain() {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
  glDeleteFramebuffers(static_cast<GLsizei>(framebuffers_.size()),
                       framebuffers_.data());
}

bool ES3PlatformSwapChain::Initialize() {
  WTF_SCOPE0("ES3PlatformSwapChain#Initialize");
  ES3PlatformContext::ExclusiveLock context_lock(platform_context_);

  // Query the initial surface size.
  size_ = platform_context_->QuerySize();

  image_create_params_.format = available_pixel_formats_[0];
  image_create_params_.size = Size3D(size_);

  // Allocate framebuffers we'll use for copying.
  framebuffers_.resize(image_count());
  glGenFramebuffers(static_cast<GLsizei>(framebuffers_.size()),
                    framebuffers_.data());

  // Allocate initial images.
  image_views_.resize(image_count());
  pending_image_presents_.resize(image_count());
  pending_acquire_fences_.resize(image_count());
  available_images_semaphore_ =
      Semaphore::Create(image_count() * 2, image_count() * 2);
  auto resize_result = ResizeWithContext(size_);
  switch (resize_result) {
    case ResizeResult::kSuccess:
      break;
    case ResizeResult::kOutOfMemory:
      LOG(ERROR) << "Failed swap chain init: out of memory";
      return false;
    case ResizeResult::kDeviceLost:
      LOG(ERROR) << "Failed swap chain init: device lost";
      return false;
  }

  // Set swap mode.
  switch (present_mode_) {
    case PresentMode::kImmediate:
      platform_context_->SetSwapBehavior(
          ES3PlatformContext::SwapBehavior::kImmediate);
      break;
    case PresentMode::kLowLatency:
      platform_context_->SetSwapBehavior(
          ES3PlatformContext::SwapBehavior::kSynchronizeAndTear);
      break;
    case PresentMode::kFifo:
      platform_context_->SetSwapBehavior(
          ES3PlatformContext::SwapBehavior::kSynchronize);
      break;
  }

  return true;
}

SwapChain::ResizeResult ES3PlatformSwapChain::Resize(Size2D new_size) {
  WTF_SCOPE0("ES3PlatformSwapChain#Resize");
  ES3PlatformContext::ExclusiveLock context_lock(platform_context_);
  return ResizeWithContext(new_size);
}

SwapChain::ResizeResult ES3PlatformSwapChain::ResizeWithContext(
    Size2D new_size) {
  WTF_SCOPE0("ES3PlatformSwapChain#ResizeWithContext");

  // TODO(benvanik): move this to the queue? won't have to worry about events.
  std::lock_guard<std::mutex> lock_guard(mutex_);

  // Recreate the underlying surface.
  auto recreate_surface_result = platform_context_->RecreateSurface(new_size);
  switch (recreate_surface_result) {
    case ES3PlatformContext::RecreateSurfaceResult::kSuccess:
      // OK.
      break;
    case ES3PlatformContext::RecreateSurfaceResult::kInvalidTarget:
      LOG(ERROR) << "Failed to recreate swap chain surface; invalid target";
      return ResizeResult::kDeviceLost;
    case ES3PlatformContext::RecreateSurfaceResult::kOutOfMemory:
      LOG(ERROR) << "Failed to recreate swap chain surface; out of memory";
      return ResizeResult::kOutOfMemory;
    case ES3PlatformContext::RecreateSurfaceResult::kDeviceLost:
      LOG(ERROR) << "Failed to recreate swap chain surface; device lost";
      return ResizeResult::kDeviceLost;
  }

  // Query the new size, as it may be different than requested.
  size_ = platform_context_->QuerySize();
  image_create_params_.size = Size3D(size_);

  auto usage_mask = Image::Usage::kTransferSource | Image::Usage::kSampled |
                    Image::Usage::kColorAttachment |
                    Image::Usage::kInputAttachment;

  // Resize all images by recreating them.
  for (int i = 0; i < image_views_.size(); ++i) {
    image_views_[i].reset();
  }
  for (int i = 0; i < image_views_.size(); ++i) {
    // Allocate image.
    ref_ptr<Image> image;
    auto result =
        memory_heap_->AllocateImage(image_create_params_, usage_mask, &image);
    DCHECK_EQ(result, MemoryHeap::AllocationResult::kSuccess);
    if (!image) {
      return ResizeResult::kOutOfMemory;
    }

    // Get a view for the target format.
    image_views_[i] =
        image->CreateView(Image::Type::k2D, image_create_params_.format);
  }

  return ResizeResult::kSuccess;
}

SwapChain::AcquireResult ES3PlatformSwapChain::AcquireNextImage(
    std::chrono::milliseconds timeout_millis,
    ref_ptr<QueueFence> signal_queue_fence,
    ref_ptr<ImageView>* out_image_view) {
  WTF_SCOPE0("ES3PlatformSwapChain#AcquireNextImage");

  std::chrono::milliseconds start_time_millis =
      SystemClock::default_clock()->now_millis();

  // Reserve an image index.
  // We'll either use one that is clean or one that does not yet have a waiter.
  int image_index = -1;
  while (image_index == -1) {
    std::chrono::milliseconds elapsed_time_millis =
        SystemClock::default_clock()->now_millis() - start_time_millis;
    if (elapsed_time_millis >= timeout_millis) {
      // Timed out trying.
      return AcquireResult::kTimeout;
    }

    // Wait until at least one image is available.
    // We only wait for the time remaining if we've already tried a bit.
    std::chrono::milliseconds try_timeout_millis =
        timeout_millis - elapsed_time_millis;
    if (Thread::Wait(available_images_semaphore_, try_timeout_millis) !=
        Thread::WaitResult::kSuccess) {
      return AcquireResult::kTimeout;
    }

    std::lock_guard<std::mutex> lock_guard(mutex_);
    // If a discard is pending we'll just fail the acquisition.
    if (is_discard_pending_) {
      available_images_semaphore_->Release(1);
      LOG(WARNING) << "Attempted to acquire an image from the swapchain with a "
                      "discard pending";
      return AcquireResult::kDiscardPending;
    }

    // Prepare the next image.
    // TODO(benvanik): implement other modes (like skipping/etc).
    // First scan for clean images.
    for (int i = 0; i < image_views_.size(); ++i) {
      if (pending_image_presents_[i] == false) {
        // Image is clean - no pending present or waiting acquire.
        // Allow the caller to use it immediately.
        image_index = i;
        pending_image_presents_[i] = true;
        auto context_lock =
            ES3PlatformContext::LockTransientContext(platform_context_);
        signal_queue_fence.As<ES3QueueFence>()->Signal();
        break;
      }
    }
    if (image_index == -1) {
      // No clean images available, try to reserve an in-flight one.
      for (int i = 0; i < image_views_.size(); ++i) {
        if (!pending_acquire_fences_[i]) {
          // Image is in-flight but doesn't yet have a waiter. Reserve it.
          image_index = i;
          pending_acquire_fences_[i] = signal_queue_fence.As<ES3QueueFence>();
          break;
        }
      }
    }
  }
  *out_image_view = image_views_[image_index];

  return AcquireResult::kSuccess;
}

SwapChain::PresentResult ES3PlatformSwapChain::PresentImage(
    ref_ptr<QueueFence> wait_queue_fence, ref_ptr<ImageView> image_view,
    std::chrono::milliseconds present_time_utc_millis) {
  WTF_SCOPE0("ES3PlatformSwapChain#PresentImage");

  int image_index = -1;
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);

    // Map image view back to our index.
    for (int i = 0; i < image_views_.size(); ++i) {
      if (image_views_[i] == image_view) {
        image_index = i;
        break;
      }
    }
    DCHECK_NE(image_index, -1);
    if (image_index == -1) {
      LOG(ERROR)
          << "Attempted to present an image not acquired from the swap chain";
      return PresentResult::kDeviceLost;
    }

    if (is_discard_pending_ && pending_image_presents_[image_index]) {
      // A discard is pending so ignore the present request.
      auto context_lock =
          ES3PlatformContext::LockTransientContext(platform_context_);
      MarkPresentComplete(image_index);
      return PresentResult::kDiscardPending;
    }
  }

  // Query current size from the context.
  Size2D surface_size = control_->size();
  bool resize_required = surface_size != size_;

  // Submit present request to the context queue.
  auto self = ref_ptr<ES3PlatformSwapChain>(this);
  auto self_token = MoveToLambda(self);
  present_queue_->EnqueueCallback(platform_context_, {wait_queue_fence},
                                  [self_token, surface_size, image_index,
                                   image_view, present_time_utc_millis]() {
                                    self_token.value->PerformPresent(
                                        surface_size, image_index, image_view,
                                        present_time_utc_millis);
                                  },
                                  {}, nullptr);

  return resize_required ? PresentResult::kResizeRequired
                         : PresentResult::kSuccess;
}

void ES3PlatformSwapChain::PerformPresent(
    Size2D surface_size, int image_index, ref_ptr<ImageView> image_view,
    std::chrono::milliseconds present_time_utc_millis) {
  WTF_SCOPE0("ES3PlatformSwapChain#PerformPresent");

  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_discard_pending_) {
      // A discard is pending, so avoid doing the present and just pretend it
      // completed.
      MarkPresentComplete(image_index);
      return;
    }
  }

  // Map image view back to a GL framebuffer.
  GLuint framebuffer_id = framebuffers_[image_index];
  GLuint texture_id = image_view->image().As<ES3Image>()->texture_id();
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

  // Invalidate the read framebuffer, as we no longer need its contents.
  // TODO(benvanik): verify we can eat the contents.
  GLenum read_invalidate_attachments[] = {
      GL_COLOR_ATTACHMENT0,
  };
  glInvalidateFramebuffer(GL_READ_FRAMEBUFFER,
                          count_of(read_invalidate_attachments),
                          read_invalidate_attachments);

  // Detach framebuffer texture to ensure it's not in use on the read frame
  // buffer. This may be required by certain impls due to multi-context use.
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, 0, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  if (!platform_context_->SwapBuffers(present_time_utc_millis)) {
    LOG(ERROR) << "Platform SwapBuffers failed";
  }

  // Invalidate the default framebuffer now that we've swapped.
  GLenum draw_invalidate_attachments[] = {
      GL_COLOR,
  };
  glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER,
                          count_of(draw_invalidate_attachments),
                          draw_invalidate_attachments);

  // Mark the image as available.
  std::lock_guard<std::mutex> lock_guard(mutex_);
  MarkPresentComplete(image_index);
}

void ES3PlatformSwapChain::MarkPresentComplete(int image_index) {
  if (pending_acquire_fences_[image_index]) {
    // A present is still pending until the pending acquire presents.
    pending_acquire_fences_[image_index]->Signal();
    pending_acquire_fences_[image_index].reset();
  } else {
    pending_image_presents_[image_index] = false;
  }
  available_images_semaphore_->Release(1);
}

void ES3PlatformSwapChain::DiscardPendingPresents() {
  WTF_SCOPE0("ES3PlatformSwapChain#DiscardPendingPresents");

  // Set discard flag so future acquire/present requests will abort immediately.
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    is_discard_pending_ = true;
  }

  // Wait for all presents to complete or abort.
  for (int i = 0; i < image_count() * 2; ++i) {
    Thread::Wait(available_images_semaphore_);
  }

  // Done with the discard; from this point on others can acquire images.
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    is_discard_pending_ = false;
  }

  // Release all images for reuse.
  available_images_semaphore_->Release(4);
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
