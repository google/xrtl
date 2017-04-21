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

#include "xrtl/ui/window.h"

#include "xrtl/base/debugging.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/testing/gtest.h"
#include "xrtl/ui/testing/mock_control_listener.h"

namespace xrtl {
namespace ui {
namespace {

// NOTE: most behavior is implemented in controls so this just tests the window
// specific logic. See control_test for more exhaustive tests.

// How long we wait before timing out on control events.
std::chrono::milliseconds GetWaitTimeout() {
  return debugging::is_debugger_attached() ? std::chrono::seconds(60)
                                           : std::chrono::milliseconds(1000);
}

class WindowTest : public ::testing::Test {
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
ref_ptr<MessageLoop> WindowTest::message_loop_;

// Tests that a window can be allocated but not created.
TEST_F(WindowTest, Uncreated) {
  auto window = Window::Create(message_loop_);
  EXPECT_EQ(message_loop_, window->message_loop());
  EXPECT_EQ("", window->title());
  EXPECT_NE(nullptr, window->root_control());
  auto control = window->root_control();
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

// Tests basic window lifecycle by opening and closing it.
TEST_F(WindowTest, OpenClose) {
  auto window = Window::Create(message_loop_);

  EXPECT_NE(nullptr, window->root_control());
  auto control = window->root_control();
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  Thread::Wait(window->Open());
  ASSERT_TRUE(control->platform_handle());

  ASSERT_NE(0, control->platform_handle());
  EXPECT_EQ(Control::State::kCreated, control->state());
  EXPECT_TRUE(control->is_active());
  EXPECT_FALSE(control->is_suspended());
  EXPECT_TRUE(control->is_focused());
  EXPECT_EQ(Size2D(128, 128), control->bounds().size);
  EXPECT_EQ(gfx::rgba8_t(0, 0, 0, 0), control->background_color());
  EXPECT_TRUE(control->is_cursor_visible());

  Thread::Wait(window->Close());
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests the content control lifecycle during window open/close.
TEST_F(WindowTest, OpenCloseEvents) {
  InSequence dummy;

  auto window = Window::Create(message_loop_);
  EXPECT_NE(nullptr, window->root_control());
  auto control = window->root_control();
  MockControlListener listener;
  control->set_listener(&listener);
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // Window creation:
  EXPECT_CALL(listener, OnCreating(control)).Times(1);
  EXPECT_CALL(listener, OnCreated(control)).Times(1);
  EXPECT_CALL(listener, OnSuspendChanged(control, false)).Times(1);
  EXPECT_CALL(listener, OnResized(control, _)).Times(AtLeast(1));
  EXPECT_CALL(listener, OnFocusChanged(control, true)).Times(1);
  // Window destruction:
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

  // A redundant open should be a no-op.
  ASSERT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(window->Open()));

  ASSERT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({window->Close(), listener.destroyed_counter}));
  EXPECT_EQ(Control::State::kDestroyed, control->state());

  // A redundant close should be a no-op.
  ASSERT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(window->Close()));
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

// Tests setting the window title.
TEST_F(WindowTest, Title) {
  auto window = Window::Create(message_loop_);

  EXPECT_NE(nullptr, window->root_control());
  auto control = window->root_control();
  EXPECT_EQ(0, control->platform_handle());
  EXPECT_EQ(Control::State::kDestroyed, control->state());
  EXPECT_EQ("", window->title());

  window->set_title("Title1");
  EXPECT_EQ("Title1", window->title());

  Thread::Wait(window->Open());
  ASSERT_TRUE(control->platform_handle());

  ASSERT_NE(0, control->platform_handle());
  EXPECT_EQ(Control::State::kCreated, control->state());
  EXPECT_EQ("Title1", window->title());

  window->set_title("Title2");
  EXPECT_EQ("Title2", window->title());

  Thread::Wait(window->Close());
  EXPECT_EQ(Control::State::kDestroyed, control->state());
}

}  // namespace
}  // namespace ui
}  // namespace xrtl
