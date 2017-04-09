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

#include <chrono>

#include "xrtl/base/macros.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

class Win32SystemClock : public SystemClock {
 public:
  Win32SystemClock() {
    // GetSystemTimePreciseAsFileTime function is only available in the latest
    // versions of Windows. For that reason, we try to look it up in
    // kernel32.dll at runtime and use an alternative option if the function
    // is not available.
    HMODULE module = ::GetModuleHandle("kernel32.dll");
    if (module) {
      GetSystemTimePreciseAsFileTime_ =
          reinterpret_cast<FnGetSystemTimePreciseAsFileTime>(
              ::GetProcAddress(module, "GetSystemTimePreciseAsFileTime"));
    }

    // Set timebase used for relative timing and get QPC frequency.
    // The frequency should not change for the life of the process.
    ::QueryPerformanceFrequency(&qpc_frequency_);
    ::QueryPerformanceCounter(&qpc_timebase_);
  }

  std::chrono::microseconds now_utc_micros() override {
    // If GetSystemTimePreciseAsFileTime is not available we fall back to the
    // (likely) millisecond-resolution chrono implementation.
    if (!GetSystemTimePreciseAsFileTime_) {
      return std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch());
    }

    FILETIME system_time;
    GetSystemTimePreciseAsFileTime_(&system_time);

    constexpr int64_t kUnixEpochStartTicks = 116444736000000000i64;
    constexpr int64_t kFtToMicroSec = 10;
    LARGE_INTEGER li;
    li.LowPart = system_time.dwLowDateTime;
    li.HighPart = system_time.dwHighDateTime;
    // Subtract unix epoch start
    li.QuadPart -= kUnixEpochStartTicks;
    // Convert to microsecs
    li.QuadPart /= kFtToMicroSec;
    return std::chrono::microseconds(li.QuadPart);
  }

  std::chrono::microseconds now_micros() override {
    LARGE_INTEGER counter;
    ::QueryPerformanceCounter(&counter);
    uint64_t elapsed_ticks = counter.QuadPart - qpc_timebase_.QuadPart;
    uint64_t elapsed_micros = elapsed_ticks * 100000;
    return std::chrono::microseconds(elapsed_micros / qpc_frequency_.QuadPart);
  }

 private:
  typedef VOID(WINAPI* FnGetSystemTimePreciseAsFileTime)(LPFILETIME);
  FnGetSystemTimePreciseAsFileTime GetSystemTimePreciseAsFileTime_ = nullptr;

  // Absolute time used as a timebase for the relative now_* calls, set on
  // clock creation.
  LARGE_INTEGER qpc_frequency_;
  LARGE_INTEGER qpc_timebase_;
};

std::unique_ptr<SystemClock> CreateWin32SystemClock() {
  return make_unique<Win32SystemClock>();
}

}  // namespace xrtl
