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

#include "xrtl/base/threading/event.h"
#include "xrtl/port/windows/base/threading/win32_wait_handle.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

namespace {

class Win32Event : public Win32WaitHandle<Event> {
 public:
  explicit Win32Event(HANDLE handle) : Win32WaitHandle(handle) {}
  ~Win32Event() override = default;

  void Set() override { ::SetEvent(handle_); }
  void Reset() override { ::ResetEvent(handle_); }
};

}  // namespace

ref_ptr<Event> Event::CreateManualResetEvent(bool initial_state) {
  return make_ref<Win32Event>(
      ::CreateEvent(nullptr, TRUE, initial_state ? TRUE : FALSE, nullptr));
}

ref_ptr<Event> Event::CreateAutoResetEvent(bool initial_state) {
  return make_ref<Win32Event>(
      ::CreateEvent(nullptr, FALSE, initial_state ? TRUE : FALSE, nullptr));
}

}  // namespace xrtl
