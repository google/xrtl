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

#ifndef XRTL_GFX_ES3_ES3_SWAP_CHAIN_H_
#define XRTL_GFX_ES3_ES3_SWAP_CHAIN_H_

#include <mutex>
#include <vector>

#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/semaphore.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/es3/es3_queue.h"
#include "xrtl/gfx/es3/es3_queue_fence.h"
#include "xrtl/gfx/memory_heap.h"
#include "xrtl/gfx/swap_chain.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3SwapChain : public SwapChain {
 public:
  static ref_ptr<ES3SwapChain> Create(
      ref_ptr<ES3PlatformContext> shared_platform_context,
      ES3Queue* present_queue, ref_ptr<MemoryHeap> memory_heap,
      ref_ptr<ui::Control> control, PresentMode present_mode, int image_count,
      ArrayView<PixelFormat> pixel_formats);

  ~ES3SwapChain() override = default;

  virtual bool Initialize() = 0;

 protected:
  ES3SwapChain(PresentMode present_mode, int image_count,
               ArrayView<PixelFormat> pixel_formats)
      : SwapChain(present_mode, image_count),
        available_pixel_formats_(pixel_formats) {}

  std::vector<PixelFormat> available_pixel_formats_;
};

class ES3PlatformSwapChain : public ES3SwapChain {
 public:
  ES3PlatformSwapChain(ES3Queue* present_queue, ref_ptr<MemoryHeap> memory_heap,
                       ref_ptr<ui::Control> control,
                       ref_ptr<ES3PlatformContext> platform_context,
                       PresentMode present_mode, int image_count,
                       ArrayView<PixelFormat> pixel_formats);
  ~ES3PlatformSwapChain() override;

  bool Initialize() override;

  ResizeResult Resize(Size2D new_size) override;

  AcquireResult AcquireNextImage(std::chrono::milliseconds timeout_millis,
                                 ref_ptr<QueueFence> signal_queue_fence,
                                 ref_ptr<ImageView>* out_image_view) override;

  PresentResult PresentImage(
      ref_ptr<QueueFence> wait_queue_fence, ref_ptr<ImageView> image_view,
      std::chrono::milliseconds present_time_utc_millis) override;

  void DiscardPendingPresents() override;

 private:
  // Performs a Resize assuming the context is currently locked.
  ResizeResult ResizeWithContext(Size2D new_size);

  // Performs a queued present; called from the context queue.
  void PerformPresent(Size2D surface_size, int image_index,
                      ref_ptr<ImageView> image_view,
                      std::chrono::milliseconds present_time_utc_millis);

  // Marks the given image as presented, possibly allowing more acquires to
  // proceed.
  // Assumes a lock on mutex_ is held.
  void MarkPresentComplete(int image_index);

  ES3Queue* present_queue_;
  ref_ptr<MemoryHeap> memory_heap_;
  ref_ptr<ui::Control> control_;
  ref_ptr<ES3PlatformContext> platform_context_;

  std::mutex mutex_;
  Image::CreateParams image_create_params_;
  std::vector<ref_ptr<ImageView>> image_views_;
  std::vector<GLuint> framebuffers_;
  std::vector<bool> pending_image_presents_;
  std::vector<ref_ptr<ES3QueueFence>> pending_acquire_fences_;
  ref_ptr<Semaphore> available_images_semaphore_;
  bool is_discard_pending_ = false;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_SWAP_CHAIN_H_
