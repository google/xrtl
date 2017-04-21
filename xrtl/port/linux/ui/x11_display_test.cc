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

#include "xrtl/port/linux/ui/x11_display.h"

#include "xrtl/testing/gtest.h"
#include "xrtl/tools/target_config/target_config.h"

// X11 is not msan friendly.
#if !defined(XRTL_CONFIG_MSAN)

namespace xrtl {
namespace ui {
namespace {

const char* kDefaultDisplay = ":0.0";

ref_ptr<EpollMessageLoop> CreateEpollMessageLoop() {
  auto message_loop = MessageLoop::Create();
  return ref_ptr<EpollMessageLoop>(
      static_cast<EpollMessageLoop*>(message_loop.get()));
}

// Tests attempting to connect to an invalid X server.
TEST(X11DisplayTest, InvalidConnection) {
  ref_ptr<EpollMessageLoop> message_loop = CreateEpollMessageLoop();

  ref_ptr<X11Display> display = X11Display::Connect("NOTVALID", message_loop);
  EXPECT_EQ(nullptr, display);

  Thread::Wait(message_loop->Exit());
}

// Tests a simple connect/disconnect.
TEST(X11DisplayTest, Connect) {
  ref_ptr<EpollMessageLoop> message_loop = CreateEpollMessageLoop();

  ref_ptr<X11Display> display =
      X11Display::Connect(kDefaultDisplay, message_loop);
  EXPECT_NE(nullptr, display);
  EXPECT_EQ(kDefaultDisplay, display->placement());
  EXPECT_EQ(message_loop, display->message_loop());
  EXPECT_NE(nullptr, display->display_handle());
  display.reset();

  Thread::Wait(message_loop->Exit());
}

// Tests connecting to the same server multiple times.
TEST(X11DisplayTest, ConnectSameServer) {
  ref_ptr<EpollMessageLoop> message_loop = CreateEpollMessageLoop();

  // Connecting to the same while still alive should give hte same display.
  ref_ptr<X11Display> display_1 =
      X11Display::Connect(kDefaultDisplay, message_loop);
  EXPECT_NE(nullptr, display_1);
  ref_ptr<X11Display> display_2 =
      X11Display::Connect(kDefaultDisplay, message_loop);
  EXPECT_NE(nullptr, display_2);
  EXPECT_EQ(display_1, display_2);
  EXPECT_EQ(display_1->display_handle(), display_2->display_handle());
  X11Display* previous_ptr = display_1.get();
  display_2.reset();
  display_1.reset();

  // Malloc a bit so that we don't get the same display pointer.
  // This is... shady.
  std::vector<uint8_t> dummy(sizeof(X11Display));

  // Try once more - should get a new display ptr.
  ref_ptr<X11Display> display_3 =
      X11Display::Connect(kDefaultDisplay, message_loop);
  EXPECT_NE(nullptr, display_3);
  EXPECT_NE(previous_ptr, display_3.get());
  display_3.reset();

  Thread::Wait(message_loop->Exit());

  dummy.clear();
}

// Tests connecting to the same server with differing message loops give
// different displays.
TEST(X11DisplayTest, ConnectSameDiffLoops) {
  ref_ptr<EpollMessageLoop> message_loop_1 = CreateEpollMessageLoop();
  ref_ptr<EpollMessageLoop> message_loop_2 = CreateEpollMessageLoop();

  ref_ptr<X11Display> display_1 =
      X11Display::Connect(kDefaultDisplay, message_loop_1);
  EXPECT_NE(nullptr, display_1);
  ref_ptr<X11Display> display_2 =
      X11Display::Connect(kDefaultDisplay, message_loop_2);
  EXPECT_NE(nullptr, display_2);
  EXPECT_NE(display_1, display_2);
  EXPECT_NE(display_1->display_handle(), display_2->display_handle());
  display_2.reset();
  display_1.reset();

  Thread::Wait(message_loop_1->Exit());
  Thread::Wait(message_loop_2->Exit());
}

// Tests adding and removing listeners.
TEST(X11DisplayTest, Listeners) {
  ref_ptr<EpollMessageLoop> message_loop = CreateEpollMessageLoop();

  ref_ptr<X11Display> display =
      X11Display::Connect(kDefaultDisplay, message_loop);
  EXPECT_NE(nullptr, display);

  // Add a new listener.
  struct MyListener : public X11Display::Listener {
    bool OnXEvent(XEvent* x_event) override { return true; }
  };
  MyListener listener_1;
  display->AddWindowListener(&listener_1, PointerWindow);

  // Remove the listener.
  display->RemoveWindowListener(&listener_1);

  // Removing again should be safe/no-op.
  display->RemoveWindowListener(&listener_1);

  Thread::Wait(message_loop->Exit());
}

// TODO(benvanik): a way to test event dispatch. It's covered by the
//                 control/window tests, but would be nice to verify here.

}  // namespace
}  // namespace ui
}  // namespace xrtl

#endif  // !XRTL_CONFIG_MSAN
