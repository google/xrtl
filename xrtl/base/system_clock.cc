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

#include "xrtl/base/macros.h"

namespace xrtl {

SystemClock* SystemClock::default_clock_ = nullptr;

SystemClock* SystemClock::default_clock() {
  return default_clock_ ? default_clock_ : logging_clock();
}

void SystemClock::set_default_clock(SystemClock* clock) {
  default_clock_ = clock;
}

SystemClock* SystemClock::logging_clock() {
  static SystemClock* clock = Create().release();
  return clock;
}

}  // namespace xrtl
