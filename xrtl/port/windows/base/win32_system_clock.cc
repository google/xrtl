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

#include "xrtl/port/windows/base/win32_system_clock.h"

#include "xrtl/base/macros.h"

namespace xrtl {

class Win32SystemClock : public SystemClock {
 public:
  Win32SystemClock() {
    // TODO(benvanik): win32 clock.
  }

  uint64_t now_utc_micros() override {
    // TODO(benvanik): win32 clock.
    return 0;
  }

  uint64_t now_micros() override {
    // TODO(benvanik): win32 clock.
    return 0;
  }
};

std::unique_ptr<SystemClock> CreateWin32SystemClock() {
  return make_unique<Win32SystemClock>();
}

}  // namespace xrtl
