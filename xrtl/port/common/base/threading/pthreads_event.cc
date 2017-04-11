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

#include <pthread.h>

#include "xrtl/base/threading/event.h"
#include "xrtl/port/common/base/threading/pthreads_wait_handle.h"

namespace xrtl {

namespace {

class PthreadsEvent : public PthreadsWaitHandle<Event> {
 public:
  explicit PthreadsEvent(bool auto_reset, bool initial_state)
      : PthreadsWaitHandle(), auto_reset_(auto_reset), state_(initial_state) {}

  void Set() override {
    pthread_mutex_lock(wait_mutex());
    if (state_) {
      // Already set - no-op.
      pthread_mutex_unlock(wait_mutex());
      return;
    }
    state_ = true;
    if (auto_reset_) {
      pthread_cond_signal(wait_cond());  // Wake all waiters.
    } else {
      pthread_cond_broadcast(wait_cond());  // Wake all waiters.
    }
    pthread_cond_broadcast(shared_multi_cond());  // Wake all multi-waiters.
    pthread_mutex_unlock(wait_mutex());
  }

  void Reset() override {
    pthread_mutex_lock(wait_mutex());
    state_ = false;
    pthread_mutex_unlock(wait_mutex());
  }

 private:
  bool Signal() override {
    Set();
    return true;
  }
  bool CheckCondition() const override { return state_; }
  void SetWaitSuccessful() override {
    if (auto_reset_) {
      state_ = false;
    }
  }

  bool auto_reset_ = false;
  std::atomic<bool> state_{false};
};

}  // namespace

ref_ptr<Event> Event::CreateManualResetEvent(bool initial_state) {
  return make_ref<PthreadsEvent>(false, initial_state);
}

ref_ptr<Event> Event::CreateAutoResetEvent(bool initial_state) {
  return make_ref<PthreadsEvent>(true, initial_state);
}

}  // namespace xrtl
