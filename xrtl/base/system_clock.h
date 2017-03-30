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

#ifndef XRTL_BASE_SYSTEM_CLOCK_H_
#define XRTL_BASE_SYSTEM_CLOCK_H_

#include <cstdint>
#include <memory>

namespace xrtl {

// Provides time query results.
// Multiple clocks may exist in a process at a time. UTC times are safe to
// compare across clock instances, but process-relative times (such as returned
// by now_millis) are only comparable with times from the same clock.
//
// Two default clocks are provided:
// - default_clock: generally useful time with no specific clock is needed.
// - logging_clock: always matches system time.
//
// Applications may define their own clocks as needed and most systems should
// prefer taking a SystemClock* as a parameter instead of using the global
// values.
class SystemClock {
 public:
  // Creates a new clock that will always match real system time.
  static std::unique_ptr<SystemClock> Create();

  // Returns a shared clock used as the default.
  // Code should either consistently accept a SystemClock* as a parameter or
  // use this value.
  static SystemClock* default_clock();
  // Overrides the default clock with the given clock instance.
  // The clock must remain alive so long as it is set as the default.
  static void set_default_clock(SystemClock* clock);

  // Returns a shared system clock used for logging.
  // This always maps to system time to ensure log timestamps can be correlated
  // between tools.
  static SystemClock* logging_clock();

  SystemClock() = default;
  virtual ~SystemClock() = default;

  // Returns the current UNIX epoch timestamp in seconds.
  inline uint32_t now_utc_secs() { return now_utc_micros() / 1000000ull; }
  // Returns the current UNIX epoch timestamp in milliseconds.
  inline uint64_t now_utc_millis() { return now_utc_micros() / 1000; }
  // Returns the current UNIX epoch timestamp in microseconds.
  virtual uint64_t now_utc_micros() = 0;

  // Returns a clock-relative timestamp in milliseconds.
  // Time base is clock creation, not wall-clock, and is not compatible
  // with the values returned from any other clock.
  inline uint32_t now_secs() { return now_micros() / 1000000; }
  // Returns a clock-relative timestamp in milliseconds.
  // Time base is clock creation, not wall-clock, and is not compatible
  // with the values returned from any other clock.
  inline uint64_t now_millis() { return now_micros() / 1000; }
  // Returns a high resolution timestamp in fractional milliseconds.
  // Time base is clock creation, not wall-clock, and is not compatible
  // with the values returned from any other clock.
  inline double now_millis_highp() {
    uint64_t now_micros_value = now_micros();
    return static_cast<double>(now_micros_value) / 1000.0;
  }
  // Returns a clock-relative timestamp in microseconds.
  // Time base is clock creation, not wall-clock, and is not compatible
  // with the values returned from any other clock.
  virtual uint64_t now_micros() = 0;

 private:
  // Currently specified default clock, if any.
  // Not owned and must remain alive with the caller.
  static SystemClock* default_clock_;
};

}  // namespace xrtl

#endif  // XRTL_BASE_SYSTEM_CLOCK_H_
