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

#ifndef XRTL_BASE_STOPWATCH_H_
#define XRTL_BASE_STOPWATCH_H_

#include <chrono>

#include "xrtl/base/system_clock.h"

namespace xrtl {

// A simple stopwatch for timing operations.
//
// Usage:
//   Stopwatch stopwatch;
//   // ... do expensive things.
//   total_time = stopwatch.elapsed_micros();
class Stopwatch {
 public:
  Stopwatch() : clock_(SystemClock::default_clock()) { Reset(); }
  explicit Stopwatch(SystemClock* clock) : clock_(clock) { Reset(); }

  // Resets the elapsed time to 0.
  void Reset() { timebase_ = clock_->now_micros(); }

  // Total microseconds elapsed since the last Reset.
  std::chrono::microseconds elapsed_micros() {
    return clock_->now_micros() - timebase_;
  }

 private:
  SystemClock* clock_ = nullptr;
  std::chrono::microseconds timebase_;
};

}  // namespace xrtl

#endif  // XRTL_BASE_STOPWATCH_H_
