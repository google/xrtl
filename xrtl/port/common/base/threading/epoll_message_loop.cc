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

#include "xrtl/port/common/base/threading/epoll_message_loop.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>

#include "xrtl/base/system_clock.h"
#include "xrtl/base/threading/event.h"

namespace xrtl {

namespace {

void ClearFd(int event_fd) {
  int ret;
  do {
    uint64_t val = 0;
    ret = read(event_fd, &val, sizeof(val));
  } while (ret != -1 && errno != EWOULDBLOCK);
}

// Signals an event_fd and wakes any epoll that may be waiting on it.
// Returns true if the signal was successful. If false it's possible the handle
// has been closed.
bool SignalFd(int event_fd) {
  int ret;
  do {
    uint64_t val = 1;
    ret = write(event_fd, &val, sizeof(val));
  } while (ret == -1 && errno == EINTR);
  return ret >= 0;
}

}  // namespace

ref_ptr<MessageLoop> MessageLoop::Create() {
  return make_ref<EpollMessageLoop>();
}

EpollMessageLoop::EpollMessageLoop() {
  clock_ = SystemClock::default_clock();

  // Create the local event FD, used by the thread to wake for async tasks.
  event_fd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  DCHECK_GE(event_fd_, 0);

  // Make the epoll handle that we will wait on.
  epoll_fd_ = epoll_create(1 + kMaxReaderCount);
  DCHECK_GE(epoll_fd_, 0);

  // Attach the event FD to the epoll FD.
  epoll_event read_event;
  read_event.events = EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLET;
  read_event.data.ptr = nullptr;  // Let's us know it's our own event_fd_.
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &read_event);
  DCHECK_GE(ret, 0);

  // We run a thread dedicated to the loop.
  Thread::CreateParams create_params;
  create_params.name = "EpollMessageLoop";
  thread_ = Thread::Create(create_params, [this]() {
    OnEnter();
    ThreadMain();
    OnExit();
  });
}

EpollMessageLoop::~EpollMessageLoop() {
  DCHECK(!thread_->is_current());
  DCHECK(!is_running_);

  // Should have no readers registered.
  for (auto& reader : readers_) {
    DCHECK_EQ(-1, reader.fd);
  }

  // Close the event and epoll handles, which should get invalidated in case
  // anyone tries to use them.
  if (event_fd_ != -1) {
    close(event_fd_);
    event_fd_ = -1;
  }
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
    epoll_fd_ = -1;
  }
}

void EpollMessageLoop::RegisterReader(int fd, std::function<void()> callback) {
  Reader reader;
  reader.fd = fd;
  reader.callback = std::move(callback);
  RegisterReader(std::move(reader));
}

void EpollMessageLoop::RegisterReader(Reader reader) {
  // Find empty slot and register.
  Reader* reader_slot = nullptr;
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    for (auto& test_reader_slot : readers_) {
      if (test_reader_slot.fd == -1) {
        reader_slot = &test_reader_slot;
        reader_slot->fd = reader.fd;
        reader_slot->callback = std::move(reader.callback);
        break;
      }
    }
  }
  CHECK(reader_slot) << "Too many readers";

  // Add the readers's fd to the epoll_fd.
  // We use the data ptr to point at the Reader directly so we can dispatch
  // without needing to perform a lookup. This works because we know we *only*
  // use the data ptr for Reader instances (today).
  epoll_event event;
  event.events = EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLET;
  event.data.ptr = reader_slot;
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, reader_slot->fd, &event);
  DCHECK_GE(ret, 0);
}

void EpollMessageLoop::UnregisterReader(int fd) {
  // Find and untrack.
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    for (auto& reader : readers_) {
      if (reader.fd == fd) {
        reader.fd = -1;
        reader.callback = nullptr;
      }
    }
  }

  // Clear event_fd from the epoll_fd.
  epoll_event event;  // Ignored but required.
  int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event);
  DCHECK_GE(ret, 0);
}

void EpollMessageLoop::ThreadMain() {
  DCHECK(thread_->is_current());

  // We update this each pump of the loop.
  // We start with 0 so that our first pump runs right away.
  std::chrono::milliseconds timeout_millis{0};

  while (true) {
    {
      std::lock_guard<std::recursive_mutex> lock(task_mutex_);
      if (!is_running_) {
        break;
      }
    }

    // Poll, blocking until either the timeout expires or an event is set.
    // The poll will return immediately if one or more events are already set
    // when we enter it.
    static const int kMaxEvents = 1 + kMaxReaderCount;
    epoll_event events[kMaxEvents];
    int num_events = -1;
    do {
      num_events = epoll_wait(epoll_fd_, events, ABSL_ARRAYSIZE(events),
                              timeout_millis.count());
    } while (num_events == -1 && errno == EINTR);
    DCHECK_GE(num_events, 0);

    for (int i = 0; i < num_events; ++i) {
      auto reader = reinterpret_cast<Reader*>(events[i].data.ptr);
      if (reader) {
        // Async reader callback.
        reader->callback();
      }
      {
        std::lock_guard<std::recursive_mutex> lock(task_mutex_);
        if (!is_running_) {
          break;
        }
      }
    }

    // Always pump the thread task queue.
    {
      std::lock_guard<std::recursive_mutex> lock(task_mutex_);
      if (!is_running_) {
        break;
      }
    }
    ClearFd(event_fd_);
    PumpTaskQueue();

    // Compute new timeout based on pending deferred tasks.
    // May return -1 if infinite (no tasks pending) or 0 (if immediate tasks
    // pending).
    timeout_millis = CalculateSoonestTimeoutMillis();
  }
}

void EpollMessageLoop::PumpTaskQueue() {
  DCHECK(thread_->is_current());

  std::chrono::milliseconds now_millis = clock_->now_millis();

  // Find tasks to run in batches.
  // We do this as the task queue may be manipulated while running tasks.
  // Find up to the batch count tasks waiting to execute.
  constexpr int kQueueBatchCount = 64;
  std::array<ref_ptr<Task>, kQueueBatchCount> task_batch;
  int task_count = 0;
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    for (auto task : pending_tasks_) {
      // TODO(benvanik): add slop time.
      auto expected_time_millis =
          std::chrono::milliseconds(task->platform_handle());
      if (expected_time_millis <= now_millis) {
        task_batch[task_count++] = task;
        if (task_count == kQueueBatchCount) {
          LOG(INFO)
              << "Task batch is full, spilling some tasks over to next Pump";
          break;  // Batch full for now.
        }
      }
    }
  }
  if (task_batch.empty()) {
    return;
  }

  // Issue all of the tasks we found for this batch.
  for (int i = 0; i < task_count; ++i) {
    ref_ptr<Task> task = std::move(task_batch[i]);

    // Invoke the task, if it's still alive.
    InvokeTask(task);

    // If this is a repeating task (and not killed) we need to schedule
    // another tick.
    // We set the due time to be the previous due time + the interval instead
    // of basing it off of a relative time. This hopefully keeps us from
    // drifting.
    if (task->period_millis().count()) {
      task->set_platform_handle(task->platform_handle() +
                                task->period_millis().count());
    }

    task.reset();

    // If the task exited the thread, bail now.
    {
      std::lock_guard<std::recursive_mutex> lock(task_mutex_);
      if (!is_running_) {
        return;
      }
    }
  }
}

std::chrono::milliseconds EpollMessageLoop::CalculateSoonestTimeoutMillis() {
  // Walk the pending tasks and get the earliest timeout.
  std::lock_guard<std::recursive_mutex> lock(task_mutex_);
  std::chrono::milliseconds now_millis = clock_->now_millis();
  std::chrono::milliseconds nearest_time_millis =
      std::chrono::milliseconds::max();
  for (auto task : pending_tasks_) {
    std::chrono::milliseconds expected_time_millis{task->platform_handle()};
    if (expected_time_millis <= now_millis) {
      return std::chrono::milliseconds(0);  // Fire immediately.
    } else {
      nearest_time_millis = std::min(nearest_time_millis, expected_time_millis);
    }
  }
  if (nearest_time_millis == std::chrono::milliseconds::max()) {
    // Nothing going to fire, so wait until we get signaled.
    return std::chrono::milliseconds(-1);
  } else {
    // Delta millis until the soonest event.
    return nearest_time_millis - now_millis;
  }
}

void EpollMessageLoop::MarshalSync(std::function<void()> callback) {
  if (is_loop_thread()) {
    // Can't marshal to ourselves; just run inline.
    callback();
    return;
  }

  // TODO(benvanik): make this much better. We shouldn't need to do this.
  // Right now this allocates a fence and does a lot of task plumbing. A
  // lightweight queue would be better. Or, pool fences. Or, make everything
  // async!
  auto callback_baton = MoveToLambda(callback);
  auto fence_event = Event::CreateFence();
  TaskList task_list;
  MarshalAsync(&task_list, [callback_baton, &fence_event]() {
    callback_baton.value();
    fence_event->Set();
  });

  // Wake the thread to process the call and wait for it to complete.
  Thread::Wait(fence_event);
}

void EpollMessageLoop::ScheduleTask(ref_ptr<Task> task) {
  // Compute estimated execution time, which we use for scheduling to avoid
  // drift.
  if (task->delay_millis().count()) {
    task->set_platform_handle(
        (clock_->now_millis() + task->delay_millis()).count());
  } else {
    task->set_platform_handle(0);
  }

  // Start tracking the task.
  MessageLoop::ScheduleTask(task);

  // TODO(benvanik): don't signal if not needed, such as when this is called on
  // our own thread or if there is a signal pending.
  SignalFd(event_fd_);
}

ref_ptr<WaitHandle> EpollMessageLoop::Exit() {
  // Note that the CFRunLoopStop may cause us to exit and try to delete
  // ourselves immediately, so we need to make sure we stay alive with this
  // ref_ptr.
  ref_ptr<WaitHandle> wait_handle(this);

  // Set the exit flag.
  // This flag should stop the loop on the next spin.
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    is_running_ = false;
  }

  // Signal the loop, which should check our flag ASAP.
  SignalFd(event_fd_);

  return wait_handle;
}

}  // namespace xrtl
