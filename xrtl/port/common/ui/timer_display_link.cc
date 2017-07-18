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

#include "xrtl/port/common/ui/timer_display_link.h"

#include <algorithm>
#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/base/system_clock.h"

namespace xrtl {
namespace ui {

TimerDisplayLink::TimerDisplayLink(ref_ptr<MessageLoop> message_loop)
    : message_loop_(std::move(message_loop)) {}

TimerDisplayLink::~TimerDisplayLink() {
  // Always force a full stop to ensure we are safe if a callback is executing
  // on another thread.
  TimerDisplayLink::Stop();
}

int TimerDisplayLink::max_frames_per_second() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  return max_frames_per_second_;
}

void TimerDisplayLink::set_max_frames_per_second(int max_frames_per_second) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  if (max_frames_per_second == max_frames_per_second_) {
    return;  // No-op.
  }
  max_frames_per_second_ = max_frames_per_second;
  preferred_frames_per_second_ =
      std::min(preferred_frames_per_second_, max_frames_per_second_);

  VLOG(1) << "TimerDisplayLink max fps changed to " << max_frames_per_second_;

  // If we are running we'll need to restart the timer.
  if (suspend_count_ == 0) {
    ConfigureThread(std::move(lock));
  }
}

int TimerDisplayLink::preferred_frames_per_second() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  return preferred_frames_per_second_;
}

void TimerDisplayLink::ConfigureThread(
    std::unique_lock<std::recursive_mutex> lock) {
  if (!is_active_ || suspend_count_) {
    if (thread_) {
      // Stopping the thread.
      // Since we've already set state for exiting it'll definitely not make
      // callbacks after this point, but we'll want to wait for it to exit to
      // be sure logs and such are clean.
      // We could probably keep the thread around (as we're likely to use it
      // again), but this way we save memory in cases where we are backgrounded.
      auto thread = std::move(thread_);
      thread_.reset();
      lock.unlock();
      Thread::Wait(thread);
    }
    return;
  }

  // Compute the duration between frames in microseconds. This is what we'll
  // tell the timer to do.
  if (preferred_frames_per_second_ == kLowLatency) {
    // As fast as possible.
    frame_time_micros_ = std::chrono::microseconds(0);
  } else if (preferred_frames_per_second_ == kMaxDisplayRate) {
    // Fixed at max display rate.
    frame_time_micros_ = std::chrono::microseconds(static_cast<uint64_t>(
        (1.0 / max_frames_per_second_) * 1000.0 * 1000.0));
  } else {
    // An actual FPS value.
    frame_time_micros_ = std::chrono::microseconds(static_cast<uint64_t>(
        (1.0 / preferred_frames_per_second_) * 1000.0 * 1000.0));
  }

  VLOG(1) << "DisplayLink started with rate " << preferred_frames_per_second_
          << ", interval " << frame_time_micros_.count();

  // Spin up the thread.
  Thread::CreateParams thread_create_params;
  thread_create_params.name = "TimerDisplayLink";
  thread_ = Thread::Create(thread_create_params, [this]() { TimerThread(); });
}

void TimerDisplayLink::TimerThread() {
  SystemClock* clock = SystemClock::default_clock();

  while (true) {
    // Query frame start time.
    std::chrono::microseconds timestamp_utc_micros = clock->now_utc_micros();

    // We hold this lock for the duration of the callback so as to prevent
    // the instance from being deleted from under us.
    std::chrono::microseconds frame_time_micros;
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      if (!is_active_ || suspend_count_ || !callback_) {
        // Cancelled from another thread or suspended. Bail out of the thread.
        return;
      }

      // Issue callback; note that it may call display link methods.
      callback_(timestamp_utc_micros);

      frame_time_micros = frame_time_micros_;
    }

    // Schedule another tick. We make sure to compensate for the amount of time
    // we've spent in the callback.
    std::chrono::microseconds callback_duration_micros =
        clock->now_utc_micros() - timestamp_utc_micros;
    std::chrono::microseconds delay_micros;
    if (callback_duration_micros < frame_time_micros) {
      delay_micros = frame_time_micros - callback_duration_micros;
    } else {
      // Prevent swamping the message loop by clamping to a minimum time.
      delay_micros = std::chrono::microseconds(1000);
    }

    // Wait for the remaining timeout.
    // Note that the actual time we spend delayed may be different than what
    // we ask for, so if we wanted to ensure no skew we'd be a bit smarter...
    Thread::Sleep(delay_micros);
  }
}

void TimerDisplayLink::Start(
    std::function<void(std::chrono::microseconds)> callback,
    int preferred_frames_per_second) {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  preferred_frames_per_second_ =
      std::min(preferred_frames_per_second, max_frames_per_second_);
  is_active_ = true;
  callback_ = std::move(callback);
  if (suspend_count_ == 0) {
    ConfigureThread(std::move(lock));
  }
}

void TimerDisplayLink::Stop() {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  bool was_active = is_active_;
  is_active_ = false;
  callback_ = nullptr;
  if (was_active) {
    VLOG(1) << "DisplayLink stopped";
    ConfigureThread(std::move(lock));
  }
}

void TimerDisplayLink::Suspend() {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  ++suspend_count_;
  if (is_active_ && suspend_count_ == 1) {
    VLOG(1) << "Active DisplayLink suspended";
    ConfigureThread(std::move(lock));
  }
}

void TimerDisplayLink::Resume() {
  std::unique_lock<std::recursive_mutex> lock(mutex_);
  DCHECK_GE(suspend_count_, 0);
  --suspend_count_;
  if (suspend_count_ == 0) {
    // Unsuspended; see if we need to restart.
    if (is_active_) {
      VLOG(1) << "Active DisplayLink resumed";
      ConfigureThread(std::move(lock));
    }
  }
}

}  // namespace ui
}  // namespace xrtl
