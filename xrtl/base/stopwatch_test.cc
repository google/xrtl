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

#include "xrtl/base/stopwatch.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

class ManualClock : public SystemClock {
 public:
  std::chrono::microseconds now_utc_micros() override {
    return std::chrono::microseconds(0);
  }

  std::chrono::microseconds now_micros() override {
    return std::chrono::microseconds(now_micros_);
  }
  void set_now_micros(uint64_t now_micros) { now_micros_ = now_micros; }

 private:
  uint64_t now_micros_ = 0;
};

TEST(StopwatchTest, Timing) {
  ManualClock clock;
  clock.set_now_micros(100);

  // Time a range.
  Stopwatch sw(&clock);
  EXPECT_EQ(0, sw.elapsed_micros().count());
  clock.set_now_micros(200);
  EXPECT_EQ(100, sw.elapsed_micros().count());

  // Reset the time.
  sw.Reset();
  EXPECT_EQ(0, sw.elapsed_micros().count());
  clock.set_now_micros(300);
  EXPECT_EQ(100, sw.elapsed_micros().count());
}

TEST(StopwatchTest, SystemClock) {
  // Default ctor uses system clock. This is just here for coverage.
  Stopwatch sw;
  sw.Reset();
}

}  // namespace
}  // namespace xrtl
