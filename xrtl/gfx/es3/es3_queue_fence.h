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

#ifndef XRTL_GFX_ES3_ES3_QUEUE_FENCE_H_
#define XRTL_GFX_ES3_ES3_QUEUE_FENCE_H_

#include <mutex>

#include "xrtl/base/threading/event.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/queue_fence.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// NOTE: special care is taken here as we can only get a fence object from GL
// when we issue it into the command stream, yet our API allows fences to be
// created without being signaled. We use a CPU-side Event fence to wait until
// issuing before we ever try to query/wait on GL.
class ES3QueueFence : public QueueFence {
 public:
  explicit ES3QueueFence(ref_ptr<ES3PlatformContext> platform_context);
  ~ES3QueueFence() override;

  bool IsSignaled() override;

  // Issues a glFenceSync to signal the fence in the current context command
  // stream.
  void Signal();

  WaitResult Wait(std::chrono::nanoseconds timeout_nanos) override;

  // Performs a wait on the GL server. The CPU will not block.
  // Assumes a context is active to insert the wait into.
  void WaitOnServer(std::chrono::nanoseconds timeout_nanos);

 private:
  // Waits for the fence to be created.
  // Returns kSuccess if the fence_id_ object is valid. timeout_nanos will be
  // adjusted for the remaining timeout after waiting.
  WaitResult WaitForIssue(std::chrono::nanoseconds* timeout_nanos,
                          GLsync* out_fence_id);

  ref_ptr<ES3PlatformContext> platform_context_;
  ref_ptr<Event> issued_fence_;
  std::mutex mutex_;
  GLsync fence_id_ = 0;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_QUEUE_FENCE_H_
