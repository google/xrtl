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

#ifndef XRTL_BASE_THREADING_WAIT_HANDLE_H_
#define XRTL_BASE_THREADING_WAIT_HANDLE_H_

#include "xrtl/base/ref_ptr.h"

namespace xrtl {

// Base type for waitable threading objects.
// Waitable objects can be used with the various Thread::Wait routines to delay
// execution until a condition is set.
class WaitHandle : public RefObject<WaitHandle> {
 public:
  WaitHandle(const WaitHandle&) = delete;
  WaitHandle& operator=(WaitHandle const&) = delete;
  virtual ~WaitHandle() = default;

  // Returns the native platform handle of the wait object.
  // This value is platform specific.
  virtual uintptr_t native_handle() = 0;

 protected:
  WaitHandle() = default;
};

}  // namespace xrtl

#endif  // XRTL_BASE_THREADING_WAIT_HANDLE_H_
