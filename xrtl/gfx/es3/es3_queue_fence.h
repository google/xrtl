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
#include "xrtl/gfx/es3/es3_queue_object.h"
#include "xrtl/gfx/queue_fence.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// Shoddy implementation of a QueueFence.
// The design of fences in GL is pretty jank and the implementations are even
// worse on most Android drivers (especially multi-context). We implement a
// hybrid here that allows for QueueFence to be signaled and waited on entirely
// on the CPU when used only for queue execution ordering.
//
// As GL only allocates fence IDs when the fences are signaled we take care to
// enable waiting on the fence before the ID has been allocated via a CPU-side
// issued_fence_. Only once it's been issued do we allow the GL fence waiting
// APIs to be used.
//
// For purely client-side operations, most specifically present queue/swap chain
// signaling, we support SignalClient as a way to only use the issued_fence_ and
// a flag to perform the fence operation. This is not correct behavior in non-GL
// APIs like Vulkan/Metal, but works well enough for GLs model. The gotcha is
// that if we were really waiting for issued commands to complete to synchronize
// memory it's not guaranteed to have happened. Since we primarily use this
// when swapping that's fine as a SwapBuffers pretty much synchronizes with the
// server.
class ES3QueueFence : public QueueFence, public ES3QueueObject {
 public:
  explicit ES3QueueFence(ES3ObjectLifetimeQueue* queue);
  ~ES3QueueFence() override;

  FenceState QueryState() override;

  // Issues a fence. If this is called from a thread that has a GL context it is
  // equivalent to calling SignalServer, and otherwise falls back to
  // SignalClient.
  void Signal();

  // Issues an immediate client-side signal, avoiding the GL server entirely.
  // This may be called from any thread.
  void SignalClient();

  // Issues a glFenceSync to signal the fence in the current context command
  // stream. The command stream must be flushed to ensure the fence makes it to
  // the server.
  void SignalServer() XRTL_REQUIRES_GL_CONTEXT;

  WaitResult Wait(std::chrono::nanoseconds timeout_nanos) override;

  // Performs a wait on the GL server. The CPU will not block.
  // Assumes a context is active to insert the wait into.
  void WaitOnServer(std::chrono::nanoseconds timeout_nanos)
      XRTL_REQUIRES_GL_CONTEXT;

 private:
  void Release() override;
  bool AllocateOnQueue() override;
  void DeallocateOnQueue() override;

  // Waits for the fence to be created.
  // Returns kSuccess if the fence_id_ object is valid. timeout_nanos will be
  // adjusted for the remaining timeout after waiting.
  WaitResult WaitForIssue(std::chrono::nanoseconds* timeout_nanos,
                          GLsync* out_fence_id);

  ES3ObjectLifetimeQueue* queue_;
  ref_ptr<Event> issued_fence_;
  std::mutex mutex_;
  GLsync fence_id_ = 0;
  bool is_issued_ = false;
  bool is_signaled_ = false;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_QUEUE_FENCE_H_
