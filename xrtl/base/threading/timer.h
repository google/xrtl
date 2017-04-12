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

#ifndef XRTL_BASE_THREADING_TIMER_H_
#define XRTL_BASE_THREADING_TIMER_H_

#include <chrono>
#include <functional>

#include "xrtl/base/threading/wait_handle.h"

namespace xrtl {

// A timer based on the Win32 timer object.
// Timers are waitable handles that can be used to synchronize threads and may
// optionally make callbacks when they are signalled. There are two types of
// timers: manual reset and auto reset. Manual reset timers work like manual
// reset events, with the timer firing being equivalent to Set and scheduling
// being equivalent to Reset. Auto reset timers work like auto reset events,
// with the timer firing being equivalent to Set and waiting being equivalent
// to Reset.
//
// Usage, as synchronization:
//  auto timer = Timer::CreateAutoResetTimer();
//  timer->ScheduleOnce(std::chrono::milliseconds(500));
//  Thread::Wait(timer);
//  LOG(INFO) << "Timer fired";
//
// Usage, as a user callback:
//  auto timer = Timer::CreateManualResetTimer([]() {
//    LOG(INFO) << "Timer fired";
//  });
//  timer->ScheduleRepeating(std::chrono::milliseconds(100),   // delay
//                           std::chrono::milliseconds(500));  // period
//
// Reference:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms687012(v=vs.85).aspx
class Timer : public WaitHandle {
 public:
  // Creates a timer that will remain signaled until it is scheduled again.
  // This is like a manual reset event.
  static ref_ptr<Timer> CreateManualResetTimer(std::function<void()> callback);
  static ref_ptr<Timer> CreateManualResetTimer() {
    return CreateManualResetTimer(nullptr);
  }

  // Creates a timer that will remain signaled until the first waiter, at which
  // time it will reset itself.
  // This is like an auto reset event.
  static ref_ptr<Timer> CreateAutoResetTimer(std::function<void()> callback);
  static ref_ptr<Timer> CreateAutoResetTimer() {
    return CreateAutoResetTimer(nullptr);
  }

  // Schedules the timer for a single instance signaling.
  // When the delay elapses the timer is set to signaled and the callback (if
  // specified) is issued from a random thread. In manual reset mode the timer
  // will remain signaled and in auto reset mode the timer will reset itself.
  // If the timer is already scheduled it is canceled before scheduling again
  // (without being signaled or issuing the callback).
  virtual void ScheduleOnce(std::chrono::microseconds delay) = 0;
  template <typename Rep, typename Period>
  inline void ScheduleOnce(std::chrono::duration<Rep, Period> delay) {
    ScheduleOnce(std::chrono::duration_cast<std::chrono::microseconds>(delay));
  }

  // Schedules the timer for multiple signals.
  // When the delay elapses the timer is set to signaled and the callback (if
  // specified) is issued from a random thread. In manual reset mode the timer
  // will remain signaled and in auto reset mode the timer will reset itself.
  // If the timer is already scheduled it is canceled before scheduling again
  // (without being signaled or issuing the callback).
  virtual void ScheduleRepeating(std::chrono::microseconds delay,
                                 std::chrono::microseconds period) = 0;
  template <typename Rep1, typename Period1, typename Rep2, typename Period2>
  inline void ScheduleRepeating(std::chrono::duration<Rep1, Period1> delay,
                                std::chrono::duration<Rep2, Period2> period) {
    ScheduleRepeating(
        std::chrono::duration_cast<std::chrono::microseconds>(delay),
        std::chrono::duration_cast<std::chrono::microseconds>(period));
  }

  // Cancels the timer if it is currently scheduled.
  // The timers signaled state will remain what it was and the callback will not
  // be called.
  virtual void Cancel() = 0;
};

}  // namespace xrtl

#endif  // XRTL_BASE_THREADING_TIMER_H_
