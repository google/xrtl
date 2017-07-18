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
#include "xrtl/base/threading/thread.h"
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

  bool is_accurate() override { return false; }
  int max_frames_per_second() override;
  void set_max_frames_per_second(int max_frames_per_second);
  int preferred_frames_per_second() override;

  void Start(std::function<void(std::chrono::microseconds)> callback,
             int preferred_frames_per_second) override;
  void Stop() override;

  void Suspend() override;
  void Resume() override;

 protected:
  // Configures the dedicated thread, initializing/deinitializing as required.
  void ConfigureThread(std::unique_lock<std::recursive_mutex> lock);
  // Thread entry point for the dedicated timer thread.
  void TimerThread();

  ref_ptr<MessageLoop> message_loop_;
  MessageLoop::TaskList pending_task_list_;

  std::recursive_mutex mutex_;
  int max_frames_per_second_ = 60;
  int preferred_frames_per_second_ = 0;
  std::chrono::microseconds frame_time_micros_;
  bool is_active_ = false;
  int suspend_count_ = 0;
  std::function<void(std::chrono::microseconds)> callback_;

  // Thread performing the core timer loop.
  // Callbacks will be dispatched from here.
  ref_ptr<Thread> thread_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_UI_TIMER_DISPLAY_LINK_H_
