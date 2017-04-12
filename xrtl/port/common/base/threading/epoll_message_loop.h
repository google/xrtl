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

#ifndef XRTL_PORT_COMMON_BASE_THREADING_EPOLL_MESSAGE_LOOP_H_
#define XRTL_PORT_COMMON_BASE_THREADING_EPOLL_MESSAGE_LOOP_H_

#include <array>
#include <chrono>
#include <utility>

#include "xrtl/base/threading/message_loop.h"
#include "xrtl/base/threading/thread.h"

namespace xrtl {

class SystemClock;

// epoll fd-based message loop implementation.
// This works on Linux, Android, and most other POSIX platforms.
// Though iOS supports pthreads, our implementation there uses the native
// CFRunLoop primitive to get better debugging and ObjC integration.
class EpollMessageLoop : public MessageLoop {
 public:
  EpollMessageLoop();
  ~EpollMessageLoop() override;

  bool is_loop_thread() override { return thread_->is_current(); }

  void MarshalSync(std::function<void()> callback) override;

  // Registers a file descriptor for polling on a file descriptor.
  // The callback will be called whenever fd is written until UnregisterReader
  // is called.
  void RegisterReader(int fd, std::function<void()> callback);
  void UnregisterReader(int fd);

  ref_ptr<WaitHandle> Exit() override;

  uintptr_t native_handle() override { return thread_->native_handle(); }

 private:
  struct Reader {
    int fd = -1;
    std::function<void()> callback;
  };

  // Registers a reader for polling.
  void RegisterReader(Reader reader);

  // Thread main that performs our core loop.
  void ThreadMain();
  // Pumps the thread and processes pending tasks.
  void PumpTaskQueue();

  // Returns a timeout, in millis, to wait during polling.
  // Returns -1 if infinite and any other value if there is an deferred
  // task pending at a point in the future.
  std::chrono::milliseconds CalculateSoonestTimeoutMillis();

  void ScheduleTask(ref_ptr<Task> task) override;

  // Clock used for timing events.
  SystemClock* clock_ = nullptr;

  // Thread that the message loop runs on.
  ref_ptr<Thread> thread_;

  int epoll_fd_ = -1;
  int event_fd_ = -1;

  // True to keep looping. Must be set under the lock.
  bool is_running_ = true;

  // Reader fds the poll is listening on.
  // This is a sparsely populated table - unused readers have an fd of -1.
  static constexpr int kMaxReaderCount = 31;
  std::array<Reader, kMaxReaderCount> readers_;
};

}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_BASE_THREADING_EPOLL_MESSAGE_LOOP_H_
