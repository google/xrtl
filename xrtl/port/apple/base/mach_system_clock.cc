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

#include "xrtl/port/apple/base/mach_system_clock.h"

#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#include "xrtl/base/macros.h"

namespace xrtl {

class MachSystemClock : public SystemClock {
 public:
  MachSystemClock() {
    // Initialize UTC clock.
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &calendar_clock_);

    // Query timebase info once on startup.
    mach_timebase_info(&timebase_info_);

    // Set timebase used for relative timing.
    timebase_mach_time_ = mach_absolute_time();
  }

  ~MachSystemClock() override {
    mach_port_deallocate(mach_host_self(), calendar_clock_);
  }

  std::chrono::microseconds now_utc_micros() override {
    mach_timespec_t clock_time;
    clock_get_time(calendar_clock_, &clock_time);
    return std::chrono::microseconds(static_cast<uint64_t>(clock_time.tv_sec) *
                                         1000000 +
                                     clock_time.tv_nsec / 1000);
  }

  std::chrono::microseconds now_micros() override {
    // Note that we rebase the absolute time before we scale so that we preserve
    // as many bits as possible.
    return std::chrono::microseconds(
        (mach_absolute_time() - timebase_mach_time_) * timebase_info_.numer /
        timebase_info_.denom / 1000);
  }

 private:
  // Mach clock service for getting UTC time.
  clock_serv_t calendar_clock_;
  // Info used to scale queried time values.
  mach_timebase_info_data_t timebase_info_;
  // Absolute time used as a timebase for the relative now_* calls, set on
  // clock creation. Units are as with mach_absolute_time.
  uint64_t timebase_mach_time_ = 0;
};

std::unique_ptr<SystemClock> CreateMachSystemClock() {
  return absl::make_unique<MachSystemClock>();
}

}  // namespace xrtl
