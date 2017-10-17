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

#ifndef XRTL_GFX_ES3_ES3_QUEUE_H_
#define XRTL_GFX_ES3_ES3_QUEUE_H_

#include <atomic>
#include <functional>
#include <queue>
#include <utility>

#include "absl/container/inlined_vector.h"
#include "absl/types/span.h"
#include "xrtl/base/intrusive_list.h"
#include "xrtl/base/intrusive_pool.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/gfx/command_buffer.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/es3/es3_queue_object.h"
#include "xrtl/gfx/queue_fence.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3CommandBuffer;

// A command queue for use by ES3Context.
// Each queue instance maintains its own thread and dedicated GL platform
// context that it uses for submission. The queue processes enqueued command
// buffers and callbacks in FIFO order.
class ES3Queue : public ES3ObjectLifetimeQueue {
 public:
  enum class Type {
    // Queue is used for command buffer submission.
    kCommandSubmission,
    // Queue is used for presentation. It may block for long periods of time
    // waiting on vsync and such.
    kPresentation,
  };

  ES3Queue(Type queue_type,
           ref_ptr<ES3PlatformContext> shared_platform_context);
  ~ES3Queue() override;

  // Enqueues a set of operations to be executed from the queue.
  void EnqueueCommandBuffers(
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      std::function<void()> pre_callback,
      absl::Span<const ref_ptr<CommandBuffer>> command_buffers,
      std::function<void()> post_callback,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle);

  // Enqueues a callback to be executed from the queue.
  // The provided context will be locked exclusively during the execution.
  void EnqueueContextCallback(
      ref_ptr<ES3PlatformContext> exclusive_context,
      absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
      std::function<void()> callback,
      absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
      ref_ptr<Event> signal_handle);

  // Waits until all commands in the queue have completed.
  // Returns false if the device was lost and the wait will never complete.
  bool WaitUntilIdle();

 private:
  // An entry containing the required information to sequence and execute
  // a queue request.
  struct QueueEntry : public IntrusiveLinkBase<void> {
    // Whether the queue entry can be safetly discarded during shutdown.
    bool discardable = false;

    // Context under which the command buffers/callback should run.
    // If omitted the queue context will be used.
    ref_ptr<ES3PlatformContext> exclusive_context;

    // Fences that must signal before the entry is allowed to process.
    absl::InlinedVector<ref_ptr<QueueFence>, 4> wait_queue_fences;

    // Command buffers, callback, or object operation to perform.
    // If more than one of the following is provided they are executed as:
    //   - pre_callback
    //   - all command_buffers
    //   - post_callback
    std::function<void()> pre_callback;
    absl::InlinedVector<ref_ptr<CommandBuffer>, 4> command_buffers;
    std::function<void()> post_callback;

    // Fences that will be signaled after the entry has processed.
    absl::InlinedVector<ref_ptr<QueueFence>, 4> signal_queue_fences;
    // An event that will be set after the entry has processed.
    ref_ptr<Event> signal_handle;
  };

  void EnqueueObjectCallback(
      ES3QueueObject* obj, ObjectReleaseCallback release_callback,
      std::function<void()> callback,
      ref_ptr<ES3PlatformContext> exclusive_context) override;
  bool SyncObjectCallback(
      ES3QueueObject* obj, std::function<bool()> callback,
      ref_ptr<ES3PlatformContext> exclusive_context) override;

  static void QueueThreadMain(void* param);
  void RunQueue();

  // Executes a list of command buffers against the underlying GL context.
  void ExecuteCommandBuffers(
      absl::Span<const ref_ptr<CommandBuffer>> command_buffers,
      ES3CommandBuffer* implementation_command_buffer) XRTL_REQUIRES_GL_CONTEXT;

  // Base platform context used by the parent ES3Context.
  ref_ptr<ES3PlatformContext> shared_platform_context_;

  Type queue_type_;

  // Thread running the queue. Depending on the queue type the thread may own
  // its own platform context for its lifetime.
  ref_ptr<Thread> queue_thread_;
  // Auto-reset event signaled when a new entry is enqueued.
  ref_ptr<Event> queue_work_pending_event_;
  // Auto-reset event signaled when an entry has been completed.
  ref_ptr<Event> queue_work_completed_event_;

  // Mutex that must be used to guard all queue structures.
  std::mutex queue_mutex_;
  // True so long as the queue is running. After the queue has stopped future
  // enqueued requests may not execute.
  bool queue_running_ = true;
  // True if the queue is actively executing an entry.
  std::atomic<bool> queue_executing_{false};
  // All currently pending entries.
  IntrusiveList<QueueEntry> queue_;
  // A mutex exclusively for guarding the queue entry pool.
  // We do this so that we don't block the queue while managing entries (such as
  // running down destructors/etc).
  std::recursive_mutex queue_entry_pool_mutex_;
  // A pool of entries to prevent reallocation.
  IntrusivePool<QueueEntry> queue_entry_pool_{16, 32};
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_QUEUE_H_
