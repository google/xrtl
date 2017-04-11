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

#include "xrtl/port/common/base/threading/pthreads_wait_handle.h"

namespace xrtl {

namespace {

pthread_mutex_t shared_multi_mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t shared_multi_cond_ = PTHREAD_COND_INITIALIZER;

}  // namespace

pthread_mutex_t* PthreadsWaitHandleImpl::shared_multi_mutex() {
  return &shared_multi_mutex_;
}

pthread_cond_t* PthreadsWaitHandleImpl::shared_multi_cond() {
  return &shared_multi_cond_;
}

}  // namespace xrtl
