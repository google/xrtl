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

namespace xrtl {
namespace gfx {
namespace es3 {

ES3QueueFence::ES3QueueFence(ref_ptr<ES3PlatformContext> platform_context)
    : platform_context_(std::move(platform_context)) {
  fence_id_ = 0;
  issued_fence_ = Event::CreateFence();
}

ES3QueueFence::~ES3QueueFence() {
  GLsync fence_id = 0;
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    std::swap(fence_id, fence_id_);
  }
  if (fence_id) {
    auto context_lock =
        ES3PlatformContext::LockTransientContext(platform_context_);
    glDeleteSync(fence_id);
  }
}

bool ES3QueueFence::IsSignaled() {
  std::lock_guard<std::mutex> lock_guard(mutex_);
  if (!fence_id_) {
    // Have not yet got a fence allocated.
    return false;
  }
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
  GLint value = 0;
  GLsizei value_count = 1;
  glGetSynciv(fence_id_, GL_SYNC_STATUS, sizeof(value), &value_count, &value);
  return value == GL_SIGNALED;
}

void ES3QueueFence::Signal() {
  // Issue the fence into the GL command stream.
  {
    auto context_lock =
        ES3PlatformContext::LockTransientContext(platform_context_);
    std::lock_guard<std::mutex> lock_guard(mutex_);
    DCHECK(!fence_id_);
    fence_id_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    DCHECK(fence_id_);
  }

  // Allow those waiting on the fence to be created to continue.
  issued_fence_->Set();
}

ES3QueueFence::WaitResult ES3QueueFence::Wait(
    std::chrono::nanoseconds timeout_nanos) {
  std::chrono::nanoseconds adjusted_timeout_nanos = timeout_nanos;
  GLsync fence_id = 0;
  WaitResult wait_result = WaitForIssue(&adjusted_timeout_nanos, &fence_id);
  if (wait_result != WaitResult::kSuccess) {
    return wait_result;
  }

  // Wait on the GPU to signal the fence.
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
  GLenum wait_return = glClientWaitSync(fence_id, GL_SYNC_FLUSH_COMMANDS_BIT,
                                        timeout_nanos.count());
  switch (wait_return) {
    case GL_CONDITION_SATISFIED:
    case GL_ALREADY_SIGNALED:
      return WaitResult::kSuccess;
    case GL_TIMEOUT_EXPIRED:
      return WaitResult::kTimeout;
    case GL_WAIT_FAILED:
    default:
      return WaitResult::kDeviceLost;
  }
}

void ES3QueueFence::WaitOnServer(std::chrono::nanoseconds timeout_nanos) {
  std::chrono::nanoseconds adjusted_timeout_nanos = timeout_nanos;
  GLsync fence_id = 0;
  WaitResult wait_result = WaitForIssue(&adjusted_timeout_nanos, &fence_id);
  if (wait_result != WaitResult::kSuccess) {
    return;
  }

  // NOTE: unfortunately GL does not support server timeouts so we have to pass
  //       IGNORED. We still use the timeout when waiting for issue, though.
  glWaitSync(fence_id, 0, GL_TIMEOUT_IGNORED);
}

ES3QueueFence::WaitResult ES3QueueFence::WaitForIssue(
    std::chrono::nanoseconds* timeout_nanos, GLsync* out_fence_id) {
  GLsync fence_id = 0;
  {
    std::lock_guard<std::mutex> lock_guard(mutex_);
    fence_id = fence_id_;
  }
  if (fence_id) {
    // Already have a fence object.
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
