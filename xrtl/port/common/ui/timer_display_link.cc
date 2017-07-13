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

int TimerDisplayLink::preferred_frames_per_second() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  return preferred_frames_per_second_;
}

float TimerDisplayLink::actual_frames_per_second() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  return actual_frames_per_second_;
}

void TimerDisplayLink::SetupTimer() {
  // If we already have a timer stop it first. This will overwrite it with new
  // settings.
  if (timer_task_) {
    VLOG(1) << "DisplayLink restarted; aborting previously active timer";
    timer_task_->Cancel();
    timer_task_.reset();
  }

  // Compute the duration between frames in microseconds. This is what we'll
  // tell the timer to do.
  std::chrono::microseconds frame_time_micros{static_cast<uint64_t>(
      (1.0 / preferred_frames_per_second_) * 1000.0 * 1000.0)};

  VLOG(1) << "DisplayLink started with interval " << frame_time_micros.count();

  // Schedule the timer and fire one callback immediately.
  timer_task_ = message_loop_->DeferRepeating(
      &pending_task_list_,
      [this]() {
        // We hold this lock for the duration of the callback so as to prevent
        // the instance from being deleted from under us.
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!callback_) {
          // Cancelled from another thread. Ignore.
          return;
        }

        // Query frame time. Real APIs get real times, fake APIs like us get
        // this.
        std::chrono::microseconds timestamp_utc_micros =
            SystemClock::default_clock()->now_utc_micros();

        // Issue callback; note that it may call display link methods.
        callback_(timestamp_utc_micros);
      },
      std::chrono::milliseconds(0),
      std::chrono::duration_cast<std::chrono::milliseconds>(frame_time_micros));
}

void TimerDisplayLink::Start(
    std::function<void(std::chrono::microseconds)> callback,
    int preferred_frames_per_second) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  preferred_frames_per_second_ =
      std::min(preferred_frames_per_second, max_frames_per_second_);
  if (preferred_frames_per_second_ == 0) {
    preferred_frames_per_second_ = max_frames_per_second_;
  }
  is_active_ = true;
  callback_ = std::move(callback);
  if (suspend_count_ == 0) {
    SetupTimer();
  }
}

void TimerDisplayLink::Stop() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  if (timer_task_) {
    VLOG(1) << "DisplayLink stopped";
    timer_task_->Cancel();
    timer_task_.reset();
  }
  preferred_frames_per_second_ = 0;
  is_active_ = false;
  callback_ = nullptr;
  actual_frames_per_second_ = 0.0f;
}

void TimerDisplayLink::Suspend() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  ++suspend_count_;
  if (timer_task_) {
    VLOG(1) << "Active DisplayLink suspended";
    timer_task_->Cancel();
    timer_task_.reset();
  }
  actual_frames_per_second_ = 0.0f;
}

void TimerDisplayLink::Resume() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  DCHECK_GE(suspend_count_, 0);
  --suspend_count_;
  if (suspend_count_ == 0) {
    // Unsuspended; see if we need to restart.
    if (is_active_) {
      VLOG(1) << "Active DisplayLink resumed";
      SetupTimer();
    }
  }
}

}  // namespace ui
}  // namespace xrtl
