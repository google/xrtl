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

#include <memory>

#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_command_buffer.h"
#include "xrtl/gfx/es3/es3_queue_fence.h"
#include "xrtl/gfx/util/memory_command_buffer.h"
#include "xrtl/gfx/util/memory_command_decoder.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Queue::ES3Queue(Type queue_type,
                   ref_ptr<ES3PlatformContext> shared_platform_context)
    : shared_platform_context_(std::move(shared_platform_context)),
      queue_type_(queue_type) {
  WTF_SCOPE0("ES3Queue#ctor");

  queue_work_pending_event_ = Event::CreateAutoResetEvent(false);
  queue_work_completed_event_ = Event::CreateAutoResetEvent(false);

  // Spawn the thread that will execute command buffers.
  Thread::CreateParams create_params;
  create_params.name = "ES3ContextQueueThread";
  queue_thread_ = Thread::Create(create_params, QueueThreadMain, this);
}

ES3Queue::~ES3Queue() {
  WTF_SCOPE0("ES3Queue#dtor");

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
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    absl::Span<const ref_ptr<CommandBuffer>> command_buffers,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  // Presentation queues cannot handle command buffers.
  DCHECK(queue_type_ != Type::kPresentation);
  QueueEntry* queue_entry = nullptr;
  {
    std::lock_guard<std::recursive_mutex> pool_lock(queue_entry_pool_mutex_);
    queue_entry = queue_entry_pool_.Allocate();
  }
  queue_entry->wait_queue_fences = {wait_queue_fences.begin(),
                                    wait_queue_fences.end()};
  queue_entry->command_buffers = {command_buffers.begin(),
                                  command_buffers.end()};
  queue_entry->signal_queue_fences = {signal_queue_fences.begin(),
                                      signal_queue_fences.end()};
  queue_entry->signal_handle = std::move(signal_handle);
  {
    std::lock_guard<std::mutex> queue_lock(queue_mutex_);
    DCHECK(queue_running_);
    queue_.push_back(queue_entry);
  }
  queue_work_pending_event_->Set();
}

void ES3Queue::EnqueueCallback(
    ref_ptr<ES3PlatformContext> exclusive_context,
    absl::Span<const ref_ptr<QueueFence>> wait_queue_fences,
    std::function<void()> callback,
    absl::Span<const ref_ptr<QueueFence>> signal_queue_fences,
    ref_ptr<Event> signal_handle) {
  QueueEntry* queue_entry = nullptr;
  {
    std::lock_guard<std::recursive_mutex> pool_lock(queue_entry_pool_mutex_);
    queue_entry = queue_entry_pool_.Allocate();
  }
  queue_entry->exclusive_context = std::move(exclusive_context);
  queue_entry->wait_queue_fences = {wait_queue_fences.begin(),
                                    wait_queue_fences.end()};
  queue_entry->callback = std::move(callback);
  queue_entry->signal_queue_fences = {signal_queue_fences.begin(),
                                      signal_queue_fences.end()};
  queue_entry->signal_handle = std::move(signal_handle);
  {
    std::lock_guard<std::mutex> queue_lock(queue_mutex_);
    DCHECK(queue_running_);
    queue_.push_back(queue_entry);
  }
  queue_work_pending_event_->Set();
}

void ES3Queue::EnqueueObjectCallback(
    ES3QueueObject* obj, ObjectReleaseCallback release_callback,
    std::function<void()> callback,
    ref_ptr<ES3PlatformContext> exclusive_context) {
  QueueEntry* queue_entry = nullptr;
  {
    std::lock_guard<std::recursive_mutex> pool_lock(queue_entry_pool_mutex_);
    queue_entry = queue_entry_pool_.Allocate();
  }
  queue_entry->exclusive_context = std::move(exclusive_context);
  auto callback_baton = MoveToLambda(callback);
  queue_entry->callback = [obj, release_callback, callback_baton]() {
    callback_baton.value();
    release_callback(obj);
  };
  {
    std::lock_guard<std::mutex> queue_lock(queue_mutex_);
    // NOTE: we allow queuing even if the queue is shutting down as we need to
    // ensure object operations still run.
    queue_.push_back(queue_entry);
  }
  queue_work_pending_event_->Set();
}

bool ES3Queue::SyncObjectCallback(
    ES3QueueObject* obj, std::function<bool()> callback,
    ref_ptr<ES3PlatformContext> exclusive_context) {
  WTF_SCOPE0("ES3Queue#SyncObjectCallback");
  auto signal_handle = Event::CreateFence();
  QueueEntry* queue_entry = nullptr;
  {
    std::lock_guard<std::recursive_mutex> pool_lock(queue_entry_pool_mutex_);
    queue_entry = queue_entry_pool_.Allocate();
  }
  queue_entry->exclusive_context = std::move(exclusive_context);
  auto callback_baton = MoveToLambda(callback);
  bool result = false;
  queue_entry->callback = [callback_baton, &result]() {
    result = callback_baton.value();
  };
  queue_entry->signal_handle = signal_handle;
  {
    std::lock_guard<std::mutex> queue_lock(queue_mutex_);
    // NOTE: we allow queuing even if the queue is shutting down as we need to
    // ensure object operations still run.
    queue_.push_back(queue_entry);
  }
  queue_work_pending_event_->Set();
  return Thread::Wait(signal_handle) == Thread::WaitResult::kSuccess && result;
}

bool ES3Queue::WaitUntilIdle() {
  WTF_SCOPE0("ES3Queue#WaitUntilIdle");
  while (true) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (!queue_running_ || (queue_.empty() && !queue_executing_)) {
        return true;
      }
    }
    Thread::Wait(queue_work_completed_event_);
  }
  return true;  // Unreachable (here to make GCC happy).
}

void ES3Queue::QueueThreadMain(void* param) {
  reinterpret_cast<ES3Queue*>(param)->RunQueue();
}

void ES3Queue::RunQueue() {
  // Acquire and lock the GL context we'll use to execute commands. It's only
  // ever used by this thread so it's safe to keep active forever.
  // Note that we only need this if we'll be executing commands.
  // We could also defer allocation until first use but having it here makes it
  // easier to track down GL context errors and keeps performance at runtime
  // predictable.
  ref_ptr<ES3PlatformContext> queue_context;
  if (queue_type_ != Type::kPresentation) {
    WTF_SCOPE0("ES3Queue#RunQueue:context_init");
    // TODO(benvanik): allocate a dedicated queue context on transfer queues.
    queue_context = shared_platform_context_;
    if (!queue_context) {
      LOG(FATAL) << "Unable to allocate a queue platform context";
      return;
    }
  }

  // The native command buffer that takes a recorded memory command buffer and
  // makes GL calls. We'll allocate it on demand.
  ref_ptr<ES3CommandBuffer> implementation_command_buffer;

  // Used to ensure we release our dequeued queue entry when done with it.
  auto release_entry = [this](QueueEntry* queue_entry) {
    std::lock_guard<std::recursive_mutex> lock(queue_entry_pool_mutex_);
    queue_entry_pool_.Release(queue_entry);
  };

  while (true) {
    std::unique_ptr<QueueEntry, decltype(release_entry)> queue_entry{
        nullptr, release_entry};

    {
      std::lock_guard<std::mutex> lock(queue_mutex_);

      // Attempt to dequeue an entry.
      if (!queue_.empty()) {
        queue_entry.reset(queue_.front());
        queue_.pop_front();
        queue_executing_ = true;
      } else {
        // Signal that there was no work available.
        queue_work_completed_event_->Set();
        if (!queue_running_) {
          // Queue shutting down and nothing queued - exit now.
          break;
        }
      }

      if (!queue_running_) {
        // Queue is shutting down; ignore non-critical entries.
        if (queue_entry->discardable) {
          queue_executing_ = false;
          continue;
        }
      }
    }

    // If there was no work pending wait for more work.
    if (!queue_entry) {
      Thread::Wait(queue_work_pending_event_);
      continue;
    }

    WTF_SCOPE0("ES3Queue#RunQueue:entry");

    ES3PlatformContext::ExclusiveLock exclusive_lock;
    ES3PlatformContext::ThreadLock thread_lock;
    if (queue_entry->exclusive_context) {
      // Exclusive lock on the request-provided context.
      exclusive_lock.reset(queue_entry->exclusive_context);
      if (!exclusive_lock.is_held()) {
        LOG(FATAL) << "Unable to make current the provided platform context";
      }
    } else if (queue_context) {
      // Use the queue context.
      DCHECK(queue_context);
      thread_lock.reset(queue_context);
      if (!thread_lock.is_held()) {
        LOG(FATAL) << "Unable to make current the queue platform context";
      }
    }

    // Wait on queue fences.
    for (const auto& queue_fence : queue_entry->wait_queue_fences) {
      queue_fence.As<ES3QueueFence>()->WaitOnServer(
          std::chrono::nanoseconds::max());
    }

    // Execute command buffers.
    if (!queue_entry->command_buffers.empty()) {
      if (!implementation_command_buffer) {
        implementation_command_buffer = make_ref<ES3CommandBuffer>();
      }
      ExecuteCommandBuffers(queue_entry->command_buffers,
                            implementation_command_buffer.get());
    }

    // Execute callback.
    if (queue_entry->callback) {
      queue_entry->callback();
    }

    // Signal queue fences.
    for (const auto& queue_fence : queue_entry->signal_queue_fences) {
      queue_fence.As<ES3QueueFence>()->SignalServer();
    }

    // Signal CPU event.
    if (queue_entry->signal_handle) {
      queue_entry->signal_handle->Set();
    }

    queue_executing_ = false;
    queue_entry.reset();
  }

  queue_work_completed_event_->Set();
  implementation_command_buffer.reset();
  if (queue_context) {
    queue_context->ClearCurrent();
    queue_context.reset();
  }
}

void ES3Queue::ExecuteCommandBuffers(
    absl::Span<const ref_ptr<CommandBuffer>> command_buffers,
    ES3CommandBuffer* implementation_command_buffer) {
  WTF_SCOPE0("ES3Queue#ExecuteCommandBuffers");
  ES3PlatformContext::CheckHasContextLock();

  for (const auto& command_buffer : command_buffers) {
    // Reset GL state.
    implementation_command_buffer->PrepareState();

    // Get the underlying memory command buffer stream.
    auto memory_command_buffer = command_buffer.As<util::MemoryCommandBuffer>();
    auto command_reader = memory_command_buffer->GetReader();

    // Execute the command buffer against our native GL implementation.
    util::MemoryCommandDecoder::Decode(&command_reader,
                                       implementation_command_buffer);

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
