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

#ifndef XRTL_PORT_COMMON_BASE_THREADING_PTHREADS_WAIT_HANDLE_H_
#define XRTL_PORT_COMMON_BASE_THREADING_PTHREADS_WAIT_HANDLE_H_

#include <pthread.h>

#include <chrono>
#include <utility>

#include "xrtl/base/threading/wait_handle.h"

namespace xrtl {

// Implementation so that we can access these methods and values from a
// WaitHandle pointer.
class PthreadsWaitHandleImpl {
 public:
  // Mutex+cond that makes up our wait var.
  pthread_mutex_t* wait_mutex() { return &wait_mutex_; }
  pthread_cond_t* wait_cond() { return &wait_cond_; }

  // Shared condition that is used for the multi-wait functionality.
  static pthread_mutex_t* shared_multi_mutex();
  static pthread_cond_t* shared_multi_cond();

  // Signals the handle, as Set/Release/etc.
  // May not be supported by all types.
  // Returns true if the signal succeeded.
  virtual bool Signal() { return true; }

  // Returns true if the wait handle is signaled.
  // This must be called from within a wait_mutex lock.
  virtual bool CheckCondition() const = 0;
  // Sets the condition when the wait has succeeded.
  // This must be called from within a wait_mutex lock.
  virtual void SetWaitSuccessful() = 0;

 protected:
  pthread_mutex_t wait_mutex_;
  pthread_cond_t wait_cond_;
};

// CRTP WaitHandle shim.
template <typename T>
class PthreadsWaitHandle : public T, protected PthreadsWaitHandleImpl {
 public:
  template <typename... Args>
  explicit PthreadsWaitHandle(Args&&... args) : T(std::forward<Args>(args)...) {
    pthread_mutex_init(&wait_mutex_, nullptr);
    pthread_cond_init(&wait_cond_, nullptr);
  }

  ~PthreadsWaitHandle() {
    pthread_mutex_destroy(&wait_mutex_);
    pthread_cond_destroy(&wait_cond_);
  }

  uintptr_t native_handle() override {
    return reinterpret_cast<uintptr_t>(
        static_cast<PthreadsWaitHandleImpl*>(this));
  }
};

}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_BASE_THREADING_PTHREADS_WAIT_HANDLE_H_
