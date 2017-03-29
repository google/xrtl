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

#include "xrtl/port/common/base/posix_system_clock.h"

#include <ctime>

#include "xrtl/base/macros.h"

namespace xrtl {

class PosixSystemClock : public SystemClock {
 public:
  PosixSystemClock() {
    // Set timebase used for relative timing.
    timespec tick_time;
    clock_gettime(CLOCK_MONOTONIC, &tick_time);
    timebase_micros_ = static_cast<uint64_t>(tick_time.tv_sec) * 1000000 +
                       tick_time.tv_nsec / 1000;
  }

  uint64_t now_utc_micros() override {
    timespec clock_time;
    clock_gettime(CLOCK_REALTIME, &clock_time);
    return static_cast<uint64_t>(clock_time.tv_sec) * 1000000 +
           clock_time.tv_nsec / 1000;
  }

  uint64_t now_micros() override {
    timespec tick_time;
    clock_gettime(CLOCK_MONOTONIC, &tick_time);
    uint64_t tick_time_micros =
        static_cast<uint64_t>(tick_time.tv_sec) * 1000000 +
        tick_time.tv_nsec / 1000;
    return tick_time_micros - timebase_micros_;
  }

 private:
  // Absolute time used as a timebase for the relative now_* calls, set on
  // clock creation.
  uint64_t timebase_micros_ = 0;
};

std::unique_ptr<SystemClock> CreatePosixSystemClock() {
  return make_unique<PosixSystemClock>();
}

}  // namespace xrtl
