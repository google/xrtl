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

#include "xrtl/gfx/es3/es3_queue_fence.h"

#include <utility>

#include "xrtl/base/system_clock.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3QueueFence::ES3QueueFence(ES3ObjectLifetimeQueue* queue) : queue_(queue) {
  issued_fence_ = Event::CreateFence();
  queue_->EnqueueObjectAllocation(this);
}

ES3QueueFence::~ES3QueueFence() = default;

void ES3QueueFence::Release() { queue_->EnqueueObjectDeallocation(this); }

bool ES3QueueFence::AllocateOnQueue() { return true; }

void ES3QueueFence::DeallocateOnQueue() {
  ES3PlatformContext::CheckHasContextLock();
  GLsync fence_id = 0;
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    fence_id = fence_id_;
    fence_id_ = nullptr;
  }
  if (fence_id) {
    glDeleteSync(fence_id);
  }
}

ES3QueueFence::FenceState ES3QueueFence::QueryState() {
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_signaled_) {
      return FenceState::kSignaled;
    } else if (!is_issued_ || !fence_id_) {
      // Have not yet got a fence allocated.
      return FenceState::kUnsignaled;
    }
  }

  // TODO(benvanik): optimize this by keeping a list of pending fences on the
  //                 queue and querying periodically. This marshaling kills our
  //                 'polls are fast' rule.
  WTF_SCOPE0("ES3QueueFence#QueryState:sync");
  FenceState fence_state = FenceState::kDeviceLost;
  queue_->EnqueueObjectCallbackAndWait(this, [this, &fence_state]() {
    WTF_SCOPE0("ES3QueueFence#QueryState:queue");
    ES3PlatformContext::CheckHasContextLock();
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_signaled_) {
      // We've been signaled since we were requested.
      fence_state = FenceState::kSignaled;
      return true;
    }
    GLint value = 0;
    GLsizei value_count = 1;
    glGetSynciv(fence_id_, GL_SYNC_STATUS, sizeof(value), &value_count, &value);
    fence_state =
        value == GL_SIGNALED ? FenceState::kSignaled : FenceState::kUnsignaled;
    if (fence_state == FenceState::kSignaled) {
      // Cache signaled state for future queries.
      is_signaled_ = true;
    }
    return true;
  });
  return fence_state;
}

void ES3QueueFence::Signal() {
  if (ES3PlatformContext::HasContextLock()) {
    SignalServer();
  } else {
    SignalClient();
  }
}

void ES3QueueFence::SignalClient() {
  WTF_SCOPE0("ES3QueueFence#SignalClient");
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_signaled_) {
      // Already signaled - don't redundantly signal.
      return;
    }
    // NOTE: we allow signaling here even if we've already issued the server
    // fence as the client is overriding the previous server signal and
    // indicating they want the fence to complete immediately.
    is_issued_ = true;
    is_signaled_ = true;
  }

  // Allow those waiting on the fence to be created to continue.
  // They'll immediately see the is_signaled_ flag and complete their wait.
  issued_fence_->Set();
}

void ES3QueueFence::SignalServer() {
  WTF_SCOPE0("ES3QueueFence#SignalServer");
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_issued_ || is_signaled_) {
      // Already issued or signaled - don't redundantly signal.
      return;
    }
    is_issued_ = true;
  }

  // Insert into the command queue.
  DCHECK(!fence_id_);
  fence_id_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  DCHECK(fence_id_);

  // Allow those waiting on the fence to be created to continue.
  issued_fence_->Set();
}

ES3QueueFence::WaitResult ES3QueueFence::Wait(
    std::chrono::nanoseconds timeout_nanos) {
  WTF_SCOPE0("ES3QueueFence#Wait");

  // Wait for the fence to be issued as we need a fence ID to pass to GL.
  // If the fence has already been signaled this will return immediately.
  std::chrono::nanoseconds adjusted_timeout_nanos = timeout_nanos;
  GLsync fence_id = 0;
  WaitResult wait_result = WaitForIssue(&adjusted_timeout_nanos, &fence_id);
  if (wait_result != WaitResult::kSuccess) {
    return wait_result;
  }

  {
    // Check if we were signaled already.
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_signaled_) {
      return WaitResult::kSuccess;
    }
  }
  DCHECK(fence_id);

  // Marshal to the queue to perform the wait. We never promised this would be
  // fast, and the hottest use of queue fences is command buffer ordering and
  // that uses WaitOnServer instead.
  std::chrono::microseconds start_time_micros =
      SystemClock::default_clock()->now_micros();
  wait_result = WaitResult::kDeviceLost;
  queue_->EnqueueObjectCallbackAndWait(this, [this, &wait_result, fence_id,
                                              adjusted_timeout_nanos,
                                              start_time_micros]() {
    ES3PlatformContext::CheckHasContextLock();

    // Account in the timeout for how long it took to marshal across to the
    // GL queue. If the queue is busy this may actually be a decent chunk
    // of time.
    std::chrono::nanoseconds elapsed_time_nanos =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            SystemClock::default_clock()->now_micros() - start_time_micros);
    if (elapsed_time_nanos > adjusted_timeout_nanos) {
      // Timed out before we even got to the wait. Perform a non-blocking
      // query instead so we don't block at any longer.
      GLint value = 0;
      GLsizei value_count = 1;
      glGetSynciv(fence_id, GL_SYNC_STATUS, sizeof(value), &value_count,
                  &value);
      wait_result =
          value == GL_SIGNALED ? WaitResult::kSuccess : WaitResult::kTimeout;
      return true;
    }
    std::chrono::nanoseconds readjusted_timeout_nanos =
        adjusted_timeout_nanos - elapsed_time_nanos;

    // Wait on the GPU to signal the fence.
    GLenum wait_return = glClientWaitSync(fence_id, GL_SYNC_FLUSH_COMMANDS_BIT,
                                          readjusted_timeout_nanos.count());
    switch (wait_return) {
      case GL_CONDITION_SATISFIED:
      case GL_ALREADY_SIGNALED: {
        // Flag signaled state so we can quickly query in the future.
        std::lock_guard<std::mutex> lock_guard(mutex_);
        is_signaled_ = true;
        wait_result = WaitResult::kSuccess;
        break;
      }
      case GL_TIMEOUT_EXPIRED:
        wait_result = WaitResult::kTimeout;
        break;
      case GL_WAIT_FAILED:
      default:
        wait_result = WaitResult::kDeviceLost;
        break;
    }

    return true;
  });
  return wait_result;
}

void ES3QueueFence::WaitOnServer(std::chrono::nanoseconds timeout_nanos) {
  ES3PlatformContext::CheckHasContextLock();

  ref_ptr<ES3QueueFence> self_ref{this};

  // Wait for the fence to be issued as we need a fence ID to pass to GL.
  // If the fence has already been signaled this will return immediately.
  std::chrono::nanoseconds adjusted_timeout_nanos = timeout_nanos;
  GLsync fence_id = 0;
  WaitResult wait_result = WaitForIssue(&adjusted_timeout_nanos, &fence_id);
  if (wait_result != WaitResult::kSuccess) {
    return;
  }

  {
    // Check if we were signaled already.
    std::lock_guard<std::mutex> lock_guard(mutex_);
    if (is_signaled_) {
      return;
    }
  }
  DCHECK(fence_id);

  // NOTE: unfortunately GL does not support server timeouts so we have to pass
  //       IGNORED. We still use the timeout when waiting for issue, though.
  glWaitSync(fence_id, 0, GL_TIMEOUT_IGNORED);

  {
    // Flag signaled state so we can quickly query in the future.
    std::lock_guard<std::mutex> lock_guard(mutex_);
    is_signaled_ = true;
  }
}

ES3QueueFence::WaitResult ES3QueueFence::WaitForIssue(
    std::chrono::nanoseconds* timeout_nanos, GLsync* out_fence_id) {
  GLsync fence_id = 0;
  bool is_signaled = false;
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    fence_id = fence_id_;
    is_signaled = is_signaled_;
  }
  if (is_signaled) {
    // Already signaled either on the client or server. The caller should
    // also check for this and early-exit.
    *out_fence_id = 0;
    return WaitResult::kSuccess;
  } else if (fence_id) {
    // Have been issued and have a fence object. We'll wait on it in the caller
    // with the GL wait primitives.
    *out_fence_id = fence_id;
    return WaitResult::kSuccess;
  }

  // Have not yet got allocated a fence. We need to wait for it to be issued.
  std::chrono::microseconds start_time_micros =
      SystemClock::default_clock()->now_micros();
  Thread::WaitResult wait_result = Thread::Wait(
      issued_fence_,
      std::chrono::duration_cast<std::chrono::milliseconds>(*timeout_nanos));
  switch (wait_result) {
    case Thread::WaitResult::kSuccess: {
      // Fence was issued, now we can wait for real. Adjust the timout by
      // how long we waited.
      std::chrono::nanoseconds elapsed_time_nanos =
          std::chrono::duration_cast<std::chrono::nanoseconds>(
              SystemClock::default_clock()->now_micros() - start_time_micros);
      if (elapsed_time_nanos >= *timeout_nanos) {
        // Actually timed out (due to precision issues).
        return WaitResult::kTimeout;
      }
      *timeout_nanos = *timeout_nanos - elapsed_time_nanos;
      {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        *out_fence_id = fence_id_;
      }
      return WaitResult::kSuccess;
    }
    case Thread::WaitResult::kTimeout:
      return WaitResult::kTimeout;
    case Thread::WaitResult::kError:
    default:
      return WaitResult::kDeviceLost;
  }
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
