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

#ifndef XRTL_BASE_THREADING_SEMAPHORE_H_
#define XRTL_BASE_THREADING_SEMAPHORE_H_

#include "xrtl/base/threading/wait_handle.h"

namespace xrtl {

// A semaphore based on the Win32 semaphore object.
// It maintains a count of [0, maximum_count]. Each time the semaphore is waited
// on the count is incremented and each time the semaphore is released the
// count is decremented. When the count reaches zero waits will block until the
// count is increased back to above zero. A semaphore is called signaled when
// the count is greater than zero and nonsignaled when the count is zero.
//
// Usage:
//   auto semaphore = Semaphore::Create(0, 2);
//   StartWorkerThreads(2, [&]() {
//     while (true) {
//       Thread::Wait(semaphore);
//       // ... do work.
//     }
//   });
//   // ... queue work.
//   semaphore->Release(1);
//   // ... queue work.
//   semaphore->Release(1);
//
// Reference:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms685129(v=vs.85).aspx
class Semaphore : public WaitHandle {
 public:
  // Creates a new semaphore object.
  // The initial_count must be greater than or equal to zero and less than or
  // equal to maximum_count.
  static ref_ptr<Semaphore> Create(int initial_count, int maximum_count);

  // The maximum count of the semaphore.
  int maximum_count() const { return maximum_count_; }

  // Increments the count by the given value.
  // release_count must be greater than zero.
  // Returns false if adding release_count would set the semaphore over the
  // initially specified maximum_count.
  virtual bool Release(int release_count, int* out_previous_count) = 0;
  inline bool Release(int release_count) {
    return Release(release_count, nullptr);
  }
  inline bool Release() { return Release(1, nullptr); }

 protected:
  explicit Semaphore(int maximum_count) : maximum_count_(maximum_count) {}

  int maximum_count_;
};

}  // namespace xrtl

#endif  // XRTL_BASE_THREADING_SEMAPHORE_H_
