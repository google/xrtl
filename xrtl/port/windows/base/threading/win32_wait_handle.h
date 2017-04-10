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

#ifndef XRTL_PORT_WINDOWS_BASE_THREADING_WIN32_WAIT_HANDLE_H_
#define XRTL_PORT_WINDOWS_BASE_THREADING_WIN32_WAIT_HANDLE_H_

#include <utility>

#include "xrtl/base/threading/wait_handle.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

// CRTP WaitHandle shim.
// This is used by the various waitable types on Windows to get safe HANDLE
// storage.
template <typename T>
class Win32WaitHandle : public T {
 public:
  template <typename... Args>
  explicit Win32WaitHandle(HANDLE handle, Args&&... args)
      : T(std::forward<Args>(args)...), handle_(handle) {}

  ~Win32WaitHandle() override {
    if (handle_) {
      ::CloseHandle(handle_);
      handle_ = nullptr;
    }
  }

  uintptr_t native_handle() override {
    return reinterpret_cast<uintptr_t>(handle_);
  }

 protected:
  HANDLE handle_ = nullptr;
};

}  // namespace xrtl

#endif  // XRTL_PORT_WINDOWS_BASE_THREADING_WIN32_WAIT_HANDLE_H_
