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

#ifndef XRTL_GFX_SWAP_CHAIN_H_
#define XRTL_GFX_SWAP_CHAIN_H_

#include <chrono>
#include <utility>

#include "xrtl/base/geometry.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/command_buffer.h"
#include "xrtl/gfx/image.h"
#include "xrtl/gfx/image_view.h"
#include "xrtl/gfx/pixel_format.h"
#include "xrtl/gfx/queue_fence.h"

namespace xrtl {
namespace gfx {

// Presentation swap chain.
// Manages a queue of Images that are used to present render output to a
// system surface. Multiple swap chains may exist in an application (one per
// surface), each with their own queue.
//
// At the start of a frame the application dequeues an image from the swap
// chain via AcquireNextImage. This may block waiting for images to
// become available from the system. Once returned, the image view can be used
// in an image for rendering. Any command buffer that uses the image view
// must wait on submit for the fence passed to AcquireNextImage signalling that
// the image is available for use.
//
// A the end of a frame the application enqueues the image for
// presentation in the swap chain with PresentImage. The image must have been
// transitioned to Layout::kPresentSource. After enqueuing for present the
// image must not be used until the next time it is acquired from the swap
// chain with AcquireNextImage.
//
// Usage:
//  auto command_buffer = context->CreateCommandBuffer(kRender);
//  // Acquire a new image, possibly blocking until one is ready:
//  swap_chain->AcquireNextImage(image_ready_fence, &image_view);
//  // Wrap image view in an image. Do this once and cache.
//  auto framebuffer = WrapFramebuffer(image_view);
//  // Use the framebuffer:
//  DoRendering(command_buffer, framebuffer);
//  // Submit for rendering into the framebuffer.
//  context_->Submit(image_ready_fence, command_buffer, rendered_fence);
//  // Asynchronously present the image.
//  swap_chain->PresentImage(std::move(rendered_fence),
//                           std::move(image_view));
class SwapChain : public RefObject<SwapChain> {
 public:
  virtual ~SwapChain() = default;

  // Defines the presentation queueing mode used by the swap chain.
  //
  // See section 20 here for more information:
  // https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-part-2#_Toc445674479
  enum class PresentMode {
    // Immediately present the swap chain contents.
    // This may cause tearing as the image being used to scan-out the
    // display may be replaced with a newly-enqueued images. This is the
    // classic novsync mode.
    //
    // Maps to VK_PRESENT_MODE_IMMEDIATE_KHR.
    kImmediate,
    // Queue up to 1 pending image at a time.
    // This prevents tearing but may introduce frame skips (if the compositor
    // runs slower than images are enqueued).
    //
    // Maps to VK_PRESENT_MODE_MAILBOX_KHR.
    kLowLatency,
    // Queues the frame buffers for FIFO processing.
    // This is like the classic GL present mode in that the compositor ensures
    // all images queued are displayed even if it is running slower than
    // they are being enqueued.
    //
    // Maps to VK_PRESENT_MODE_FIFO_KHR.
    kFifo,
  };

  // Presentation mode used by the compositor defining the queuing mode of the
  // swap chain.
  PresentMode present_mode() const { return present_mode_; }
  // Maximum number of images in the swap chain queue.
  // This is almost always 2 (for double-buffering).
  int image_count() const { return image_count_; }
  // Pixel format the swap chain is using.
  // This may be different than one of the suggested formats if none of them
  // were available for use.
  PixelFormat pixel_format() const { return pixel_format_; }
  // Dimensions of the swap chain images in pixels.
  Size2D size() const { return size_; }

  // Defines the result of a swap chain resize operation.
  enum class ResizeResult {
    // Resize completed successfully and the new images are available for
    // use immediately.
    kSuccess,
    // Memory was not available to allocate the new images. The old
    // images may no longer be valid. Consider this fatal to the
    // swap chain.
    kOutOfMemory,
    // The device was lost before or during resize.
    kDeviceLost,
  };

  // Resizes the images to the given dimensions.
  // The contents of the images are undefined after resizing. The queue
  // images must not currently be in use by any in-flight command buffer.
  // This may fail if memory is not available for the new frame buffers or the
  // device is lost. If it fails the contents and validity of the swap chain are
  // both undefined and it's best to fail up.
  //
  // The provided size may be ignored if the target surface requires the swap
  // chain to fill its contents. Always query the size() after a Resize to
  // ensure the proper size is used.
  virtual ResizeResult Resize(Size2D new_size) = 0;

  // Defines the result of a dequeue image operation.
  enum class AcquireResult {
    // An image was successfully dequeued.
    kSuccess,
    // The target swap chain surface has been resized and the swap chain no
    // longer matches. Resize the swap chain before continuing to render.
    kResizeRequired,
    // The specified timeout elapsed while waiting for an image to become
    // available.
    kTimeout,
    // A swap chain discard is pending and the image could not be acquired.
    kDiscardPending,
    // The device was lost before or during the wait to dequeue an image.
    kDeviceLost,
  };

  // Dequeues an image from the swap chain image pool.
  // If none is available the call may block until one is unless a timeout is
  // specified.
  //
  // On success the returned image will be in Layout::kUndefined. The wait fence
  // will be signaled asynchronously when a command buffer may be submitted that
  // uses the image as a framebuffer target.
  //
  // The returned image should be presented with PresentImage as
  // soon as possible. Do not present out of order.
  virtual AcquireResult AcquireNextImage(
      std::chrono::milliseconds timeout_millis,
      ref_ptr<QueueFence> signal_queue_fence,
      ref_ptr<ImageView>* out_image_view) = 0;
  AcquireResult AcquireNextImage(ref_ptr<QueueFence> signal_queue_fence,
                                 ref_ptr<ImageView>* out_image_view) {
    return AcquireNextImage(std::chrono::milliseconds::max(),
                            std::move(signal_queue_fence), out_image_view);
  }

  // Defines the result of an enqueue image operation.
  enum class PresentResult {
    // The image was enqueued for presentation.
    kSuccess,
    // The target swap chain surface has been resized and the swap chain no
    // longer matches. Resize the swap chain before continuing to render.
    kResizeRequired,
    // A swap chain discard is pending and the image was not presented.
    kDiscardPending,
    // The device was lost before or during the enqueue operation.
    kDeviceLost,
  };

  // Enqueues the given image for presentation on the swap chain.
  // The behavior of the enqueue operation depends on the present mode.
  // This must only be used with an image returned from the previous call to
  // AcquireNextImage.
  //
  // The provided wait fence must be used to signal that all operations that
  // use the image have completed and that the image has been transitioned to
  // Layout::kPresentSource.
  //
  // The image must not be used again until it is dequeued from
  // AcquireNextImage even if this function fails.
  //
  // If an absolute present time is specified (as SystemClock::now_utc_millis())
  // the compositor may wait to display it on the screen until that time. Not
  // all implementations support this so it should only be treated as a hint.
  //
  // Calls to this function will never block. If the caller requires that the
  // image be presented they must use Context::WaitUntilQueuesIdle.
  virtual PresentResult PresentImage(
      ref_ptr<QueueFence> wait_queue_fence, ref_ptr<ImageView> image_view,
      std::chrono::milliseconds present_time_utc_millis) = 0;
  PresentResult PresentImage(ref_ptr<QueueFence> wait_queue_fence,
                             ref_ptr<ImageView> image_view) {
    return PresentImage(std::move(wait_queue_fence), std::move(image_view),
                        std::chrono::milliseconds::min());
  }

  // Requests that all pending presents are discarded.
  // This can be used when the swap chain content is no longer useful, such as
  // when the application is being backgrounded.
  //
  // This will block if a present is in progress and after this method returns
  // no presents will occur unless more are queued. Note that all pending
  // acquire fences will be signaled but attempts to present them will fail.
  virtual void DiscardPendingPresents() = 0;

 protected:
  SwapChain(PresentMode present_mode, int image_count)
      : present_mode_(present_mode), image_count_(image_count) {}

  PresentMode present_mode_ = PresentMode::kLowLatency;
  int image_count_ = 0;
  PixelFormat pixel_format_;
  Size2D size_{0, 0};
};

std::ostream& operator<<(std::ostream& stream,
                         const SwapChain::ResizeResult& value);
std::ostream& operator<<(std::ostream& stream,
                         const SwapChain::AcquireResult& value);
std::ostream& operator<<(std::ostream& stream,
                         const SwapChain::PresentResult& value);

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_SWAP_CHAIN_H_
