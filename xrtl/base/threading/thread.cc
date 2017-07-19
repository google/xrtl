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

#include "xrtl/base/threading/thread.h"

namespace xrtl {

Thread::Thread() = default;

Thread::~Thread() = default;

void Thread::RegisterExitCallback(std::function<void()> callback) {
  std::lock_guard<std::mutex> lock(exit_mutex_);
  exit_callbacks_.push_back(std::move(callback));
}

void Thread::OnEnter() {
  // TODO(benvanik): prepare WTF on this thread.
}

void Thread::OnExit() {
  // Call exit routines in reverse order of registration.
  // NOTE: this is not re-entrant!
  {
    std::lock_guard<std::mutex> lock(exit_mutex_);
    for (auto it = exit_callbacks_.rbegin(); it != exit_callbacks_.rend();
         ++it) {
      (*it)();
    }
    exit_callbacks_.clear();
  }

  // TODO(benvanik): shutdown WTF for this thread.
}

std::ostream& operator<<(std::ostream& stream,
                         const Thread::WaitResult& value) {
  switch (value) {
    XRTL_UNREACHABLE_DEFAULT();
    case Thread::WaitResult::kSuccess:
      return stream << "WaitResult::kSuccess";
    case Thread::WaitResult::kTimeout:
      return stream << "WaitResult::kTimeout";
    case Thread::WaitResult::kError:
      return stream << "WaitResult::kError";
  }
}

}  // namespace xrtl
