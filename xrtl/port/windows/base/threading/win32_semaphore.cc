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

#include "xrtl/base/threading/semaphore.h"
#include "xrtl/port/windows/base/threading/win32_wait_handle.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

namespace {

class Win32Semaphore : public Win32WaitHandle<Semaphore> {
 public:
  Win32Semaphore(HANDLE handle, int maximum_count)
      : Win32WaitHandle(handle, maximum_count) {}

  bool Release(int release_count, int* out_previous_count) override {
    return ::ReleaseSemaphore(handle_, release_count,
                              reinterpret_cast<LPLONG>(out_previous_count)) !=
           0;
  }
};

}  // namespace

ref_ptr<Semaphore> Semaphore::Create(int initial_count, int maximum_count) {
  return make_ref<Win32Semaphore>(
      ::CreateSemaphore(nullptr, initial_count, maximum_count, nullptr),
      maximum_count);
}

}  // namespace xrtl
