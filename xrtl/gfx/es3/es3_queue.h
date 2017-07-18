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

#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/gfx/es3/es3_command_buffer.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/es3/es3_queue_fence.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// A command queue for use by ES3Context.
// Each queue instance maintains its own thread and dedicated GL platform
// context that it uses for submission. The queue processes enqueued command
// buffers and callbacks in FIFO order.
class ES3Queue {
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
  ~ES3Queue();

  // Enqueues a set of command buffers to be executed from the queue.
  void EnqueueCommandBuffers(ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
                             ArrayView<ref_ptr<CommandBuffer>> command_buffers,
                             ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
                             ref_ptr<Event> signal_handle);

  // Enqueues a callback to be executed from the queue.
  // The provided context will be locked exclusively during the execution.
  void EnqueueCallback(ref_ptr<ES3PlatformContext> exclusive_context,
                       ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
                       std::function<void()> callback,
                       ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
                       ref_ptr<Event> signal_handle);

  // Waits until all commands in the queue have completed.
  // Returns false if the device was lost and the wait will never complete.
  bool WaitUntilIdle();

 private:
  static void QueueThreadMain(void* param);
  void RunQueue();
  void ExecuteCommandBuffers(
      const std::vector<ref_ptr<CommandBuffer>>& command_buffers,
      ref_ptr<ES3CommandBuffer> implementation_command_buffer);

  // Base platform context used by the parent ES3Context.
  ref_ptr<ES3PlatformContext> shared_platform_context_;

  Type queue_type_;

  ref_ptr<Thread> queue_thread_;
  ref_ptr<Event> queue_work_pending_event_;
  ref_ptr<Event> queue_work_completed_event_;

  std::mutex queue_mutex_;
  bool queue_running_ = true;
  bool queue_executing_ = false;

  struct QueueEntry {
    ref_ptr<ES3PlatformContext> exclusive_context;
    std::vector<ref_ptr<QueueFence>> wait_queue_fences;
    std::vector<ref_ptr<CommandBuffer>> command_buffers;
    std::function<void()> callback;
    std::vector<ref_ptr<QueueFence>> signal_queue_fences;
    ref_ptr<Event> signal_handle;
    QueueEntry() = default;
    QueueEntry(ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
               ArrayView<ref_ptr<CommandBuffer>> command_buffers,
               ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
               ref_ptr<Event> signal_handle)
        : wait_queue_fences(wait_queue_fences),
          command_buffers(command_buffers),
          signal_queue_fences(signal_queue_fences),
          signal_handle(std::move(signal_handle)) {}
    QueueEntry(ref_ptr<ES3PlatformContext> exclusive_context,
               ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
               std::function<void()> callback,
               ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
               ref_ptr<Event> signal_handle)
        : exclusive_context(std::move(exclusive_context)),
          wait_queue_fences(wait_queue_fences),
          callback(std::move(callback)),
          signal_queue_fences(signal_queue_fences),
          signal_handle(std::move(signal_handle)) {}
  };

  std::queue<QueueEntry> queue_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_QUEUE_H_
