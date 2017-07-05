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

#include "xrtl/testing/demo_main.h"

#include "xrtl/base/logging.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/ui/window.h"

namespace xrtl {
namespace ui {
namespace {

class WindowDemo : private Control::Listener, private Control::InputListener {
 public:
  WindowDemo() {
    message_loop_ = MessageLoop::Create();
    done_event_ = Event::CreateFence();
  }

  ~WindowDemo() { Thread::Wait(message_loop_->Exit()); }

  ref_ptr<WaitHandle> Run() {
    window_ = Window::Create(message_loop_);
    window_->set_title("Window Demo");

    auto control = window_->root_control();
    control->set_listener(this);
    control->set_input_listener(this);
    control->set_size({640, 480});
    control->set_background_color({255, 0, 0, 255});

    Thread::Wait(window_->Open());

    return done_event_;
  }

 private:
  void OnError(ref_ptr<Control> target) override {
    LOG(INFO) << "OnError";
    done_event_->Set();
  }

  void OnCreating(ref_ptr<Control> target) override {
    LOG(INFO) << "OnCreating";
  }

  void OnCreated(ref_ptr<Control> target) override { LOG(INFO) << "OnCreated"; }

  void OnDestroying(ref_ptr<Control> target) override {
    LOG(INFO) << "OnDestroying";
  }

  void OnDestroyed(ref_ptr<Control> target) override {
    LOG(INFO) << "OnDestroyed";
    done_event_->Set();
  }

  void OnSystemThemeChanged(ref_ptr<Control> target) override {
    LOG(INFO) << "OnSystemThemeChanged";
  }

  void OnSuspendChanged(ref_ptr<Control> target, bool is_suspended) override {
    LOG(INFO) << "OnSuspendChanged: " << is_suspended;
  }

  void OnFocusChanged(ref_ptr<Control> target, bool is_focused) override {
    LOG(INFO) << "OnFocusChanged: " << is_focused;
  }

  void OnResized(ref_ptr<Control> target, Rect2D bounds) override {
    LOG(INFO) << "OnResized: " << bounds.origin.x << "," << bounds.origin.y
              << " " << bounds.size.width << "x" << bounds.size.height;
  }

  void OnKeyDown(ref_ptr<Control> target, const KeyboardEvent& ev) override {
    LOG(INFO) << "OnKeyDown: " << ev.key_code();
  }

  void OnKeyUp(ref_ptr<Control> target, const KeyboardEvent& ev) override {
    LOG(INFO) << "OnKeyUp: " << ev.key_code();
  }

  void OnKeyPress(ref_ptr<Control> target, const KeyboardEvent& ev) override {
    LOG(INFO) << "OnKeyPress: " << ev.key_code();
  }

  void OnMouseDown(ref_ptr<Control> target, const MouseEvent& ev) override {
    LOG(INFO) << "OnMouseDown: " << ev.control_offset_px().x << ","
              << ev.control_offset_px().y << " b"
              << static_cast<int>(ev.action_button());
  }

  void OnMouseUp(ref_ptr<Control> target, const MouseEvent& ev) override {
    LOG(INFO) << "OnMouseUp: " << ev.control_offset_px().x << ","
              << ev.control_offset_px().y << " b"
              << static_cast<int>(ev.action_button());
  }

  void OnMouseOut(ref_ptr<Control> target, const MouseEvent& ev) override {
    LOG(INFO) << "OnMouseOut: " << ev.control_offset_px().x << ","
              << ev.control_offset_px().y;
  }

  void OnMouseMove(ref_ptr<Control> target, const MouseEvent& ev) override {
    LOG(INFO) << "OnMouseMove: " << ev.control_offset_px().x << ","
              << ev.control_offset_px().y;
  }

  void OnMouseWheel(ref_ptr<Control> target, const MouseEvent& ev) override {
    LOG(INFO) << "OnMouseWheel: " << ev.control_offset_px().x << ","
              << ev.control_offset_px().y << " d" << ev.wheel_delta();
  }

  ref_ptr<MessageLoop> message_loop_;
  ref_ptr<Window> window_;
  ref_ptr<Event> done_event_;
};

int MyEntry(int argc, char** argv) {
  auto demo = make_unique<WindowDemo>();
  Thread::Wait(demo->Run());
  demo.reset();
  LOG(INFO) << "Clean exit!";
  return 0;
}

}  // namespace
}  // namespace ui
}  // namespace xrtl

DECLARE_ENTRY_POINT(xrtl::ui::MyEntry);
