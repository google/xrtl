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

#ifndef XRTL_GFX_QUEUE_FENCE_H_
#define XRTL_GFX_QUEUE_FENCE_H_

#include <chrono>

#include "xrtl/base/ref_ptr.h"

namespace xrtl {
namespace gfx {

// A fence that orders queue command buffer submissions.
// These may be signaled once per object. Attempting to signal an already-
// signaled fence may produce undefined results.
//
// These are device-side fences that can only be signaled from the GPU. They
// can then be used to order GPU commands by waiting on the GPU or synchronize
// the CPU by waiting for the GPU to hit the fence.
//
// QueueFence roughly maps to the follow backend concepts:
// - D3D12: ?
// - Metal: (emulated)
// - OpenGL: glFenceSync (kind of)
// - Vulkan: VkSemaphore
class QueueFence : public RefObject<QueueFence> {
 public:
  virtual ~QueueFence() = default;

  // Queries the current status of the queue fence without blocking.
  // Returns true if the fence has been signaled.
  virtual bool IsSignaled() = 0;

  // Defines the return value for QueueFence wait operations.
  enum class WaitResult {
    // Wait completed successfully either during the wait call or prior to it.
    kSuccess = 0,
    // The timeout period elapsed without the fence being signaled.
    kTimeout,
    // Wait failed because the device was lost while waiting.
    kDeviceLost,
  };

  // Blocks and waits for the fence to become signaled.
  // The timeout provided must not be infinite and will be clamped to the
  // Device::Limits::max_queue_fence_timeout_nanos value. Always handle timeout!
  virtual WaitResult Wait(std::chrono::nanoseconds timeout_nanos) = 0;

 protected:
  QueueFence() = default;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_QUEUE_FENCE_H_
