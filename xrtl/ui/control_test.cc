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

#include "xrtl/ui/control.h"

#include "xrtl/base/debugging.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/semaphore.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/testing/gtest.h"
#include "xrtl/ui/testing/mock_control_listener.h"

namespace xrtl {
namespace ui {
namespace {

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Return;

// How long we wait before timing out on control events.
std::chrono::milliseconds GetWaitTimeout() {
  return debugging::is_debugger_attached() ? std::chrono::seconds(60)
                                           : std::chrono::milliseconds(1000);
}

// Use this as a template:
struct TestListener : public Control::Listener {
  void OnError(ref_ptr<Control> target) override {}
  void OnCreating(ref_ptr<Control> target) override {}
  void OnCreated(ref_ptr<Control> target) override {}
  void OnDestroying(ref_ptr<Control> target) override {}
  void OnDestroyed(ref_ptr<Control> target) override {}
  void OnSystemThemeChanged(ref_ptr<Control> target) override {}
  void OnSuspendChanged(ref_ptr<Control> target, bool is_suspended) override {}
  void OnFocusChanged(ref_ptr<Control> target, bool is_focused) override {}
  void OnResized(ref_ptr<Control> target, Rect2D bounds) override {}
};

class ControlTest : public ::testing::Test {
 public:
  // X *really* hates cycling the display connection, so persist it.
  static void SetUpTestCase() { message_loop_ = MessageLoop::Create(); }
  static void TearDownTestCase() {
    Thread::Wait(message_loop_->Exit());
    message_loop_.reset();
  }

 protected:
  static ref_ptr<MessageLoop> message_loop_;
};
ref_ptr<MessageLoop> ControlTest::message_loop_;

// Tests that a control can be allocated but not created.
TEST_F(ControlTest, Uncreated) {
  auto control = Control::Create(message_loop_);
  EXPECT_EQ(message_loop_, control->message_loop());
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());
  EXPECT_FALSE(control->is_active());
  EXPECT_FALSE(control->is_suspended());
  EXPECT_TRUE(control->is_focused());
  EXPECT_EQ(Rect2D(0, 0, 128, 128), control->bounds());
  EXPECT_EQ(gfx::rgba8_t(0, 0, 0, 0), control->background_color());
  EXPECT_TRUE(control->is_cursor_visible());
}

// Tests basic control lifecycle by creating and destroying it.
TEST_F(ControlTest, CreateDestroy) {
  auto control = Control::Create(message_loop_);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  Thread::Wait(control->Create());
  ASSERT_TRUE(control->platform_handle());

  ASSERT_NE(0, control->platform_handle());
  EXPECT_EQ(Control::State::kCreated, control->state());
  EXPECT_TRUE(control->is_active());
  EXPECT_FALSE(control->is_suspended());
  EXPECT_TRUE(control->is_focused());
  EXPECT_EQ(Size2D(128, 128), control->bounds().size);
  EXPECT_EQ(gfx::rgba8_t(0, 0, 0, 0), control->background_color());
  EXPECT_TRUE(control->is_cursor_visible());

  Thread::Wait(control->Destroy());
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests the content control lifecycle during control create/destroy.
// This asserts a lot of the default values, default events, and event ordering.
// To ensure cross-platform code works reliably we need to enforce some level of
// determinism in our events.
TEST_F(ControlTest, CreateDestroyEvents) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation:
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());
  EXPECT_EQ(Control::State::kCreated, control->state());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Wait for destroy.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests suspending.
TEST_F(ControlTest, SetSuspended) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation:
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // set_suspended(true):
  EXPECT_CALL(listener, OnSuspendChanged(control, true)).Times(1);
  EXPECT_CALL(listener, OnFocusChanged(control, false)).Times(1);
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Set suspended and wait for the 2 events.
  control->set_suspended(true);
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({listener.suspend_changed_counter,
                             listener.focus_changed_counter},
                            GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests toggling the suspend state of a control on and off.
TEST_F(ControlTest, SetSuspendedToggle) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation:
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // set_suspended(true):
  EXPECT_CALL(listener, OnSuspendChanged(control, true)).Times(1);
  EXPECT_CALL(listener, OnFocusChanged(control, false)).Times(1);
  // set_suspended(false):
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Set suspended and wait for the 2 events.
  control->set_suspended(true);
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({listener.suspend_changed_counter,
                             listener.focus_changed_counter},
                            GetWaitTimeout()));

  // No-op set suspended again.
  control->set_suspended(true);

  // Set suspended and wait for the 2 events.
  control->set_suspended(false);
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({listener.suspend_changed_counter,
                             listener.focus_changed_counter},
                            GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests changing focus.
TEST_F(ControlTest, SetFocused) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation:
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // set_focused(false):
  EXPECT_CALL(listener, OnFocusChanged(control, false)).Times(1);
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Lose focused and wait for the event.
  control->set_focused(false);
  ASSERT_EQ(
      Thread::WaitResult::kSuccess,
      Thread::WaitAll({listener.focus_changed_counter}, GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests toggling the focus state of a control on and off.
TEST_F(ControlTest, SetFocusedToggle) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation:
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // set_focused(false):
  EXPECT_CALL(listener, OnFocusChanged(control, false)).Times(1);
  // set_focused(true):
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Lose focus and wait for the event.
  control->set_focused(false);
  ASSERT_EQ(
      Thread::WaitResult::kSuccess,
      Thread::WaitAll({listener.focus_changed_counter}, GetWaitTimeout()));

  // No-op set focus again.
  control->set_focused(false);

  // Give focus and wait for the event.
  control->set_focused(true);
  ASSERT_EQ(
      Thread::WaitResult::kSuccess,
      Thread::WaitAll({listener.focus_changed_counter}, GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests setting the focus state initially to false.
TEST_F(ControlTest, SetFocusInitial) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation (unfocused):
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, false)).Times(1);
  // set_focused(true):
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  control->set_focused(false);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Take focus and wait for the event.
  control->set_focused(true);
  ASSERT_EQ(
      Thread::WaitResult::kSuccess,
      Thread::WaitAll({listener.focus_changed_counter}, GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests setting the bounds of a control.
TEST_F(ControlTest, SetBounds) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation (at default bounds):
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
// set_bounds(new_bounds):
#if defined(XRTL_PLATFORM_LINUX)
  // TODO(benvanik): figure out how to get accurate offset in multimon.
  EXPECT_CALL(listener, OnResized(control, _)).Times(1);
#else
  EXPECT_CALL(listener, OnResized(control, Rect2D(150, 150, 300, 300)))
      .Times(1);
#endif  // XRTL_PLATFORM_LINUX
// set_size(new_size):
#if defined(XRTL_PLATFORM_LINUX)
  // TODO(benvanik): figure out how to get accurate offset in multimon.
  EXPECT_CALL(listener, OnResized(control, _)).Times(1);
#else
  EXPECT_CALL(listener, OnResized(control, Rect2D(150, 150, 400, 400)))
      .Times(1);
#endif  // XRTL_PLATFORM_LINUX
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Resize.
  control->set_bounds(Rect2D(150, 150, 300, 300));
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({listener.resized_counter}, GetWaitTimeout()));

  // Redundant resize should *not* fire an event.
  control->set_bounds(Rect2D(150, 150, 300, 300));

  // Resize using the set_size helper.
  control->set_size(Size2D(400, 400));
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({listener.resized_counter}, GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests setting the initial bounds of a control.
TEST_F(ControlTest, SetBoundsInitial) {
  InSequence dummy;

  auto control = Control::Create(message_loop_);
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Control creation (at our initial bounds):
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
#if defined(XRTL_PLATFORM_LINUX)
  // TODO(benvanik): figure out how to get accurate offset in multimon.
  EXPECT_CALL(listener, OnResized(control, _)).Times(1);
#else
  EXPECT_CALL(listener, OnResized(control, Rect2D(100, 100, 200, 200)))
      .Times(AtLeast(1));
#endif  // XRTL_PLATFORM_LINUX
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
// set_bounds(new_bounds):
#if defined(XRTL_PLATFORM_LINUX)
  // TODO(benvanik): figure out how to get accurate offset in multimon.
  EXPECT_CALL(listener, OnResized(control, _)).Times(1);
#else
  EXPECT_CALL(listener, OnResized(control, Rect2D(150, 150, 300, 300)))
      .Times(1);
#endif  // XRTL_PLATFORM_LINUX
  // Control destruction:
  EXPECT_CALL(listener, OnDestroying(control)).Times(1);
  EXPECT_CALL(listener, OnDestroyed(control)).Times(1);

  control->set_bounds(Rect2D(100, 100, 200, 200));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Create(), listener.created_counter}));
  ASSERT_TRUE(control->platform_handle());

  // Wait for the initial events.
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll(
                {listener.suspend_changed_counter, listener.resized_counter,
                 listener.focus_changed_counter},
                GetWaitTimeout()));

  // Resize.
  control->set_bounds(Rect2D(150, 150, 300, 300));
  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({listener.resized_counter}, GetWaitTimeout()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({control->Destroy(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// TODO(benvanik): set background color.

// TODO(benvanik): set cursor visible.

// TODO(benvanik): invalidate, when we have painting support.

}  // namespace
}  // namespace ui
}  // namespace xrtl
