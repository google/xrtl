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

#ifndef XRTL_PORT_COMMON_UI_TIMER_DISPLAY_LINK_H_
#define XRTL_PORT_COMMON_UI_TIMER_DISPLAY_LINK_H_

#include <mutex>

#include "xrtl/base/threading/message_loop.h"
#include "xrtl/ui/display_link.h"

namespace xrtl {
namespace ui {

// A simple implementation of DisplayLink based on the MessageLoop timer.
// This is *not* accurate and can cause tearing. It's mainly provided for
// platforms that do not have their own native DisplayLink-alike API.
class TimerDisplayLink : public DisplayLink {
 public:
  explicit TimerDisplayLink(ref_ptr<MessageLoop> message_loop);
  ~TimerDisplayLink();

  int max_frames_per_second() override;
  int preferred_frames_per_second() override;
  float actual_frames_per_second() override;

  void Start(std::function<void(std::chrono::microseconds)> callback,
             int preferred_frames_per_second) override;
  void Stop() override;

  void Suspend() override;
  void Resume() override;

 protected:
  // Assumes the lock is held.
  void SetupTimer();

  ref_ptr<MessageLoop> message_loop_;
  MessageLoop::TaskList pending_task_list_;

  std::recursive_mutex mutex_;
  int max_frames_per_second_ = 60;
  int preferred_frames_per_second_ = 0;
  float actual_frames_per_second_ = 0.0f;
  bool is_active_ = false;
  int suspend_count_ = 0;
  std::function<void(std::chrono::microseconds)> callback_;

  // TODO(benvanik): replace with Timer once that's implemented; the MessageLoop
  //                 task impl here has only millisecond granularity.
  ref_ptr<MessageLoop::Task> timer_task_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_UI_TIMER_DISPLAY_LINK_H_
