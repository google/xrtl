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

#ifndef XRTL_UI_TESTING_MOCK_CONTROL_LISTENER_H_
#define XRTL_UI_TESTING_MOCK_CONTROL_LISTENER_H_

#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/semaphore.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/testing/gtest.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace ui {

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;

class MockControlListener : public Control::Listener {
 public:
  MockControlListener() {
    ON_CALL(*this, OnError(_))
        .WillByDefault(Invoke(this, &MockControlListener::SignalOnError));
    ON_CALL(*this, OnCreating(_))
        .WillByDefault(Invoke(this, &MockControlListener::SignalOnCreating));
    ON_CALL(*this, OnCreated(_))
        .WillByDefault(Invoke(this, &MockControlListener::SignalOnCreated));
    ON_CALL(*this, OnDestroying(_))
        .WillByDefault(Invoke(this, &MockControlListener::SignalOnDestroying));
    ON_CALL(*this, OnDestroyed(_))
        .WillByDefault(Invoke(this, &MockControlListener::SignalOnDestroyed));
    ON_CALL(*this, OnSystemThemeChanged(_))
        .WillByDefault(
            Invoke(this, &MockControlListener::SignalOnSystemThemeChanged));
    ON_CALL(*this, OnSuspendChanged(_, _))
        .WillByDefault(
            Invoke(this, &MockControlListener::SignalOnSuspendChanged));
    ON_CALL(*this, OnFocusChanged(_, _))
        .WillByDefault(
            Invoke(this, &MockControlListener::SignalOnFocusChanged));
    ON_CALL(*this, OnResized(_, _))
        .WillByDefault(Invoke(this, &MockControlListener::SignalOnResized));
  }

  void SignalOnError(ref_ptr<Control> target) {
    LOG(INFO) << "OnError";
    error_counter->Release(1);
  }
  void SignalOnCreating(ref_ptr<Control> target) {
    LOG(INFO) << "OnCreating";
    creating_counter->Release(1);
  }
  void SignalOnCreated(ref_ptr<Control> target) {
    LOG(INFO) << "OnCreated";
    created_counter->Release(1);
  }
  void SignalOnDestroying(ref_ptr<Control> target) {
    LOG(INFO) << "OnDestroying";
    destroying_counter->Release(1);
  }
  void SignalOnDestroyed(ref_ptr<Control> target) {
    LOG(INFO) << "OnDestroyed";
    destroyed_counter->Release(1);
  }
  void SignalOnSystemThemeChanged(ref_ptr<Control> target) {
    LOG(INFO) << "OnSystemThemeChanged";
    system_theme_changed_counter->Release(1);
  }
  void SignalOnSuspendChanged(ref_ptr<Control> target, bool is_suspended) {
    LOG(INFO) << "OnSuspendChanged: " << is_suspended;
    suspend_changed_counter->Release(1);
  }
  void SignalOnFocusChanged(ref_ptr<Control> target, bool is_focused) {
    LOG(INFO) << "OnFocusChanged: " << is_focused;
    focus_changed_counter->Release(1);
  }
  void SignalOnResized(ref_ptr<Control> target, Rect2D bounds) {
    LOG(INFO) << "OnResized: " << bounds.origin.x << "," << bounds.origin.y
              << " " << bounds.size.width << "x" << bounds.size.height;
    resized_counter->Release(1);
  }

  MOCK_METHOD1(OnError, void(ref_ptr<Control>));
  MOCK_METHOD1(OnCreating, void(ref_ptr<Control>));
  MOCK_METHOD1(OnCreated, void(ref_ptr<Control>));
  MOCK_METHOD1(OnDestroying, void(ref_ptr<Control>));
  MOCK_METHOD1(OnDestroyed, void(ref_ptr<Control>));
  MOCK_METHOD1(OnSystemThemeChanged, void(ref_ptr<Control>));
  MOCK_METHOD2(OnSuspendChanged, void(ref_ptr<Control>, bool));
  MOCK_METHOD2(OnFocusChanged, void(ref_ptr<Control>, bool));
  MOCK_METHOD2(OnResized, void(ref_ptr<Control>, Rect2D));

  ref_ptr<Semaphore> error_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> creating_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> created_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> destroying_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> destroyed_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> system_theme_changed_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> suspend_changed_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> focus_changed_counter = Semaphore::Create(0, 100);
  ref_ptr<Semaphore> resized_counter = Semaphore::Create(0, 100);
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_TESTING_MOCK_CONTROL_LISTENER_H_
