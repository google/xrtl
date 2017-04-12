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

#include "xrtl/base/threading/message_loop.h"

#include <utility>

#include "xrtl/base/logging.h"

namespace xrtl {

MessageLoop::MessageLoop() = default;

MessageLoop::~MessageLoop() {
  // Should have had all MessageLoop::TaskLists destroyed already.
  DCHECK(pending_tasks_.empty());
}

void MessageLoop::OnEnter() {}

void MessageLoop::OnExit() {
  // Cancel all pending tasks.
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    exiting_ = true;
    while (!pending_tasks_.empty()) {
      ref_ptr<Task> task = pending_tasks_.front();
      task->Cancel();
    }
  }

  // Call exit routines now that we have zero remaining timers/deferreds.
  // If we did this first it's possible that the timer/deferred cleanup could
  // access things the exit routines deallocate.
  for (auto it = exit_callbacks_.rbegin(); it != exit_callbacks_.rend(); ++it) {
    (*it)();
  }
  exit_callbacks_.clear();

  // Shouldn't be able to queue up more work while exiting.
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    DCHECK(pending_tasks_.empty());
  }
}

void MessageLoop::MarshalAsync(MessageLoop::TaskList* pending_task_list,
                               std::function<void()> callback) {
  DeferTask(pending_task_list, std::move(callback),
            std::chrono::milliseconds(0), std::chrono::milliseconds(0));
}

ref_ptr<MessageLoop::Task> MessageLoop::Defer(
    MessageLoop::TaskList* pending_task_list, std::function<void()> callback) {
  return DeferTask(pending_task_list, std::move(callback),
                   std::chrono::milliseconds(0), std::chrono::milliseconds(0));
}

ref_ptr<MessageLoop::Task> MessageLoop::Defer(
    MessageLoop::TaskList* pending_task_list, std::function<void()> callback,
    std::chrono::milliseconds delay_millis) {
  return DeferTask(pending_task_list, std::move(callback), delay_millis,
                   std::chrono::milliseconds(0));
}

ref_ptr<MessageLoop::Task> MessageLoop::DeferRepeating(
    MessageLoop::TaskList* pending_task_list, std::function<void()> callback,
    std::chrono::milliseconds period_millis) {
  return DeferTask(pending_task_list, std::move(callback), period_millis,
                   period_millis);
}

ref_ptr<MessageLoop::Task> MessageLoop::DeferRepeating(
    MessageLoop::TaskList* pending_task_list, std::function<void()> callback,
    std::chrono::milliseconds delay_millis,
    std::chrono::milliseconds period_millis) {
  return DeferTask(pending_task_list, std::move(callback), delay_millis,
                   period_millis);
}

ref_ptr<MessageLoop::Task> MessageLoop::DeferTask(
    MessageLoop::TaskList* pending_task_list, std::function<void()> callback,
    std::chrono::milliseconds delay_millis,
    std::chrono::milliseconds period_millis) {
  // Allocate a task from the pool, initialize it, and schedule it.
  ref_ptr<Task> task;
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    if (exiting_) {
      LOG(WARNING)
          << "Message loop tasks were scheduled by exit routines during "
             "MessageLoop::OnExit; don't do that!";
      return nullptr;
    }
    task = ref_ptr<Task>(task_pool_.Allocate());
  }
  task->Initialize(this, pending_task_list, delay_millis, period_millis,
                   std::move(callback));
  return task;
}

void MessageLoop::ScheduleTask(ref_ptr<Task> task) {
  std::lock_guard<std::recursive_mutex> lock(task_mutex_);
  DCHECK(!pending_tasks_.contains(task));
  pending_tasks_.push_back(std::move(task));
}

void MessageLoop::DescheduleTask(ref_ptr<Task> task) {
  std::lock_guard<std::recursive_mutex> lock(task_mutex_);
  DCHECK(pending_tasks_.contains(task));
  pending_tasks_.erase(task);  // May delete the task!
}

bool MessageLoop::IsTaskScheduled(Task* task) {
  std::lock_guard<std::recursive_mutex> lock(task_mutex_);
  return pending_tasks_.contains(task);
}

void MessageLoop::InvokeTask(ref_ptr<Task> task) {
  DCHECK(is_loop_thread());
  task->Invoke();
}

void MessageLoop::DeallocateTask(Task* task) {
  std::lock_guard<std::recursive_mutex> lock(task_mutex_);
  task_pool_.Release(task);
}

void MessageLoop::RegisterExitCallback(std::function<void()> callback) {
  exit_callbacks_.push_back(std::move(callback));
}

MessageLoop::Task::Task() = default;

MessageLoop::Task::~Task() = default;

void MessageLoop::Task::Delete(Task* task) {
  if (task->message_loop()) {
    // Return the task to the thread pool.
    task->message_loop()->DeallocateTask(task);
  } else {
    // Delete, as no pool to return to.
    delete task;
  }
}

void MessageLoop::Task::Initialize(MessageLoop* message_loop,
                                   MessageLoop::TaskList* pending_task_list,
                                   std::chrono::milliseconds delay_millis,
                                   std::chrono::milliseconds period_millis,
                                   std::function<void()> callback) {
  message_loop_ = message_loop;
  pending_task_list_ = pending_task_list;
  delay_millis_ = delay_millis;
  period_millis_ = period_millis;
  callback_ = std::move(callback);

  is_alive_ = true;

  // Register with the task list so we can be canceled easily later.
  if (pending_task_list_) {
    pending_task_list_->RegisterTask(this);
  }

  // Schedule with the message loop. It will retain us until descheduled.
  message_loop_->ScheduleTask(ref_ptr<Task>(this));
}

void MessageLoop::Task::Invoke() {
  // Keep ourselves alive for the duration of the invocation as the callback
  // may release us.
  ref_ptr<Task> self(this);

  {
    // Take the lock, which lets us know that we can't be canceling while we are
    // executing.
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!is_alive_) {
      // No-op (but wasteful).
      return;
    }

    // Perform callback, ensuring we bring the callback onto the stack.
    // This dance makes it safe to cancel in the callback.
    auto callback = std::move(callback_);
    callback_ = nullptr;
    callback();
    if (is_alive_) {
      callback_ = std::move(callback);
    }
  }

  // If one-shot, clean ourselves up.
  // NOTE: we may have already been canceled by the callback, but that's ok.
  if (!period_millis_.count()) {
    Cancel();
  }
}

void MessageLoop::Task::Cancel() {
  // Keep us alive during the cancel.
  ref_ptr<Task> self(this);

  {
    // Take the lock, which lets us know that we can't be executing while we are
    // canceling.
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!is_alive_) {
      // No-op.
      return;
    }

    // Mark as dead.
    is_alive_ = false;

    // Remove from the task list we are registered with, if any.
    if (pending_task_list_) {
      pending_task_list_->UnregisterTask(this);
    }
    pending_task_list_ = nullptr;

    // Cleanup callback (may release resources).
    callback_ = nullptr;
  }

  // Deschedule from the thread.
  message_loop_->DescheduleTask(ref_ptr<Task>(this));

  // NOTE: we may be deallocated after this function returns if the thread held
  //       the last reference.
}

MessageLoop::TaskList::TaskList() = default;

MessageLoop::TaskList::~TaskList() {
  while (true) {
    ref_ptr<Task> task;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (list_.empty()) {
        break;
      }
      task = ref_ptr<Task>(list_.front());
    }

    // Cancel the task, which will loop back and unregister from list_.
    task->Cancel();

    // NOTE: task may be deleted here!
    task.reset();
  }
}

void MessageLoop::TaskList::RegisterTask(Task* task) {
  std::lock_guard<std::mutex> lock(mutex_);
  DCHECK(!list_.contains(task));
  list_.push_back(task);
}

void MessageLoop::TaskList::UnregisterTask(Task* task) {
  std::lock_guard<std::mutex> lock(mutex_);
  DCHECK(list_.contains(task));
  list_.erase(task);
}

}  // namespace xrtl
