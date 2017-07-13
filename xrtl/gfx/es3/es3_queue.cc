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

#include "xrtl/gfx/es3/es3_queue.h"

#include "xrtl/base/tracing.h"
#include "xrtl/gfx/util/memory_command_buffer.h"
#include "xrtl/gfx/util/memory_command_decoder.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Queue::ES3Queue(ref_ptr<ES3PlatformContext> shared_platform_context)
    : shared_platform_context_(std::move(shared_platform_context)) {
  queue_work_pending_event_ = Event::CreateAutoResetEvent(false);
  queue_work_completed_event_ = Event::CreateAutoResetEvent(false);

  // Spawn the thread that will execute command buffers.
  Thread::CreateParams create_params;
  create_params.name = "ES3ContextQueueThread";
  queue_thread_ = Thread::Create(create_params, QueueThreadMain, this);
}

ES3Queue::~ES3Queue() {
  // Join with queue thread.
  if (queue_thread_) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      queue_running_ = false;
    }
    queue_work_pending_event_->Set();
    queue_thread_->Join();
  }
}

void ES3Queue::EnqueueCommandBuffers(
    ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
    ArrayView<ref_ptr<CommandBuffer>> command_buffers,
    ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  DCHECK(queue_running_);
  queue_.emplace(wait_queue_fences, command_buffers, signal_queue_fences,
                 signal_handle);
  queue_work_pending_event_->Set();
}

void ES3Queue::EnqueueCallback(
    ArrayView<ref_ptr<QueueFence>> wait_queue_fences,
    std::function<void()> callback,
    ArrayView<ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  DCHECK(queue_running_);
  queue_.emplace(wait_queue_fences, std::move(callback), signal_queue_fences,
                 signal_handle);
  queue_work_pending_event_->Set();
}

void ES3Queue::WaitUntilIdle() {
  WTF_SCOPE0("ES3Queue#WaitUntilIdle");
  while (true) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (!queue_running_ || (queue_.empty() && !queue_executing_)) {
        return;
      }
    }
    Thread::Wait(queue_work_completed_event_);
  }
}

void ES3Queue::QueueThreadMain(void* param) {
  reinterpret_cast<ES3Queue*>(param)->RunQueue();
}

void ES3Queue::RunQueue() {
  // Acquire and lock the GL context we'll use to execute commands. It's only
  // ever used by this thread so it's safe to keep active forever.
  auto queue_context =
      ES3PlatformContext::AcquireThreadContext(shared_platform_context_);
  if (!queue_context) {
    LOG(FATAL) << "Unable to allocate a queue platform context";
    return;
  }

  // Create the native command buffer that takes a recorded memory command
  // buffer and makes GL calls.
  auto implementation_command_buffer = make_ref<ES3CommandBuffer>();

  while (true) {
    QueueEntry queue_entry;
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (!queue_running_) {
        // Queue is shutting down; exit thread.
        break;
      }

      // Attempt to dequeue a command buffer set.
      if (!queue_.empty()) {
        queue_entry = std::move(queue_.front());
        queue_.pop();
        queue_executing_ = true;
      } else {
        // Signal that there was no work available.
        queue_work_completed_event_->Set();
      }
    }

    // If there was no work pending wait for more work.
    if (!queue_executing_) {
      Thread::Wait(queue_work_pending_event_);
      continue;
    }

    // Wait on queue fences.
    if (!queue_entry.wait_queue_fences.empty()) {
      std::vector<ref_ptr<WaitHandle>> wait_events;
      wait_events.reserve(queue_entry.wait_queue_fences.size());
      for (const auto& queue_fence : queue_entry.wait_queue_fences) {
        wait_events.push_back(queue_fence.As<ES3QueueFence>()->event());
      }
      if (Thread::WaitAll(wait_events) != Thread::WaitResult::kSuccess) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_executing_ = false;
        break;
      }
    }

    // Execute command buffers.
    if (!queue_entry.command_buffers.empty()) {
      ES3PlatformContext::ThreadLock context_lock(
          ES3PlatformContext::AcquireThreadContext(shared_platform_context_));
      if (!context_lock.is_held()) {
        LOG(FATAL) << "Unable to make current the queue platform context";
      }
      ExecuteCommandBuffers(queue_entry.command_buffers,
                            implementation_command_buffer);
    }

    // Execute callback.
    if (queue_entry.callback) {
      queue_entry.callback();
    }

    // Signal queue fences.
    for (const auto& queue_fence : queue_entry.signal_queue_fences) {
      queue_fence.As<ES3QueueFence>()->event()->Set();
    }

    // Signal CPU event.
    if (queue_entry.signal_handle) {
      queue_entry.signal_handle->Set();
    }

    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_executing_ = false;
  }

  queue_work_completed_event_->Set();
  implementation_command_buffer.reset();
  queue_context.reset();
  ES3PlatformContext::ReleaseThreadContext();
}

void ES3Queue::ExecuteCommandBuffers(
    const std::vector<ref_ptr<CommandBuffer>>& command_buffers,
    ref_ptr<ES3CommandBuffer> implementation_command_buffer) {
  for (const auto& command_buffer : command_buffers) {
    // Reset GL state.
    implementation_command_buffer->PrepareState();

    // Get the underlying memory command buffer stream.
    auto memory_command_buffer = command_buffer.As<util::MemoryCommandBuffer>();
    auto command_reader = memory_command_buffer->GetReader();

    // Execute the command buffer against our native GL implementation.
    util::MemoryCommandDecoder::Decode(&command_reader,
                                       implementation_command_buffer.get());

    // Reset our execution command buffer to clear all state.
    // This ensures that the next time we start using it the state is clean
    // (as expected by command buffers).
    implementation_command_buffer->Reset();

    // Reset the command buffer now that we have executed it. This should
    // release any resources kept alive exclusively by the command buffer.
    memory_command_buffer->Reset();
  }

  // TODO(benvanik): avoid? need to flush to ensure presents on other threads
  //                 see the outputs, probably.
  glFlush();
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
