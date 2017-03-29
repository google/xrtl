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

#include "xrtl/base/system_clock.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

TEST(SystemClockTest, SharedClocks) {
  EXPECT_NE(nullptr, SystemClock::default_clock());
  EXPECT_NE(nullptr, SystemClock::logging_clock());
}

TEST(SystemClockTest, DefaultClock) {
  EXPECT_NE(nullptr, SystemClock::default_clock());

  // Override clock with our own.
  auto my_clock = SystemClock::Create();
  EXPECT_NE(my_clock.get(), SystemClock::default_clock());
  SystemClock::set_default_clock(my_clock.get());
  EXPECT_EQ(my_clock.get(), SystemClock::default_clock());

  // Reset clock back to the system default.
  SystemClock::set_default_clock(nullptr);
  EXPECT_NE(nullptr, SystemClock::default_clock());
  EXPECT_NE(my_clock.get(), SystemClock::default_clock());
}

TEST(SystemClockTest, PlatformClock) {
  EXPECT_NE(nullptr, SystemClock::default_clock());
  SystemClock* platform_clock = SystemClock::default_clock();

  // Some valid range (2015-2030).
  EXPECT_LT(1420070400, platform_clock->now_utc_secs());
  EXPECT_GT(1893456000, platform_clock->now_utc_secs());

  // Some valid range (should be process relative, so small).
  EXPECT_LT(0, platform_clock->now_micros());
  EXPECT_GT(100000, platform_clock->now_micros());
}

class ManualClock : public SystemClock {
 public:
  uint64_t now_utc_micros() override { return now_utc_micros_; }
  void set_now_utc_micros(uint64_t now_utc_micros) {
    now_utc_micros_ = now_utc_micros;
  }

  uint64_t now_micros() override { return now_micros_; }
  void set_now_micros(uint64_t now_micros) { now_micros_ = now_micros; }

 private:
  uint64_t now_utc_micros_ = 0;
  uint64_t now_micros_ = 0;
};

TEST(SystemClockTest, NowUtcUnits) {
  ManualClock manual_clock;
  manual_clock.set_now_utc_micros(1490657899300667);
  EXPECT_EQ(1490657899300667, manual_clock.now_utc_micros());
  EXPECT_EQ(1490657899300, manual_clock.now_utc_millis());
  EXPECT_EQ(1490657899, manual_clock.now_utc_secs());
}

TEST(SystemClockTest, NowUnits) {
  ManualClock manual_clock;
  manual_clock.set_now_micros(1490657899300667);
  EXPECT_EQ(1490657899300667, manual_clock.now_micros());
  EXPECT_EQ(1490657899300, manual_clock.now_millis());
  EXPECT_EQ(1490657899, manual_clock.now_secs());
}

}  // namespace
}  // namespace xrtl
