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

#include <atomic>

#include "xrtl/base/threading/semaphore.h"
#include "xrtl/port/common/base/threading/pthreads_wait_handle.h"

namespace xrtl {

namespace {

class PthreadsSemaphore : public PthreadsWaitHandle<Semaphore> {
 public:
  PthreadsSemaphore(int initial_count, int maximum_count)
      : PthreadsWaitHandle(maximum_count), count_(initial_count) {}

  bool Release(int release_count, int* out_previous_count) override {
    pthread_mutex_lock(wait_mutex());
    if (count_ + 1 > maximum_count_) {
      pthread_mutex_unlock(wait_mutex());
      return false;
    }
    int previous_count = count_.fetch_add(release_count);
    if (previous_count + release_count > maximum_count_) {
      // Fail if we go over.
      count_.fetch_add(-release_count);
      pthread_mutex_unlock(wait_mutex());
      return false;
    }
    if (out_previous_count) {
      *out_previous_count = previous_count;
    }
    pthread_cond_signal(wait_cond());             // Wake one waiter.
    pthread_cond_broadcast(shared_multi_cond());  // Wake all multi-waiters.
    pthread_mutex_unlock(wait_mutex());
    return true;
  }

 private:
  bool Signal() override { return Release(1, nullptr); }
  bool CheckCondition() const override { return count_ > 0; }
  void SetWaitSuccessful() override { --count_; }

  std::atomic<int> count_{0};
};

}  // namespace

ref_ptr<Semaphore> Semaphore::Create(int initial_count, int maximum_count) {
  return make_ref<PthreadsSemaphore>(initial_count, maximum_count);
}

}  // namespace xrtl
