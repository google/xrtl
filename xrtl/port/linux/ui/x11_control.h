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

#ifndef XRTL_PORT_LINUX_UI_X11_CONTROL_H_
#define XRTL_PORT_LINUX_UI_X11_CONTROL_H_

#include "xrtl/base/threading/event.h"
#include "xrtl/port/common/ui/timer_display_link.h"
#include "xrtl/port/linux/ui/x11_display.h"
#include "xrtl/port/linux/ui/x11_headers.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace ui {

// X11 implementation of a control (child window).
class X11Control : public Control, private X11Display::Listener {
 public:
  // Interface used by control containers.
  class ControlContainer {
   public:
    virtual void OnChildCreated(ref_ptr<X11Control> child_control) {}
    virtual void OnChildDestroyed(ref_ptr<X11Control> child_control) {}
  };

  X11Control(ref_ptr<MessageLoop> message_loop, ControlContainer* container);
  ~X11Control() override;

  ::Display* display_handle() const;
  ::Window window_handle() const;
  PlatformHandle platform_handle() override;
  PlatformHandle platform_display_handle() override;

  State state() override;
  bool is_active() override;
  bool is_suspended() override;
  void set_suspended(bool suspended) override;
  bool is_focused() override;
  void set_focused(bool focused) override;
  Rect2D bounds() override;
  void set_bounds(Rect2D bounds) override;
  gfx::rgba8_t background_color() override;
  void set_background_color(gfx::rgba8_t background_color) override;
  bool is_cursor_visible() override;
  void set_cursor_visible(bool cursor_visible) override;

  ref_ptr<DisplayLink> display_link() override { return display_link_; }

  ref_ptr<WaitHandle> Create() override;
  ref_ptr<WaitHandle> Destroy() override;

  void Invalidate() override;

 protected:
  bool BeginCreate();
  bool EndCreate();
  bool BeginDestroy();
  bool EndDestroy();

  void InvalidateWithLock();

  Frame2D QueryFrame();
  Point2D QueryOrigin();
  enum class WindowState {
    kWithdrawn = 0,  // WithdrawnState
    kNormal = 1,     // NormalState
    kIconic = 3,     // IconicState
  };
  WindowState QueryWindowState();

  // From X11Display::Listener:
  bool OnXEvent(::XEvent* x_event) override;

  ControlContainer* container_ = nullptr;

  ref_ptr<X11Display> display_;
  ::Window window_handle_ = 0;
  uintptr_t wm_delete_window_atom_ = 0;   // WM_DELETE_WINDOW
  uintptr_t wm_state_atom_ = 0;           // WM_STATE
  uintptr_t net_active_window_atom_ = 0;  // _NET_ACTIVE_WINDOW
  uintptr_t net_frame_extents_atom_ = 0;  // _NET_FRAME_EXTENTS

  ref_ptr<Event> create_event_;
  ref_ptr<Event> destroy_event_;

  State state_ = State::kDestroyed;

  int configure_count_ = 0;

  bool is_suspended_ = false;
  bool is_focused_ = true;
  Rect2D bounds_{{0, 0}, {128, 128}};
  gfx::rgba8_t background_color_;
  bool is_cursor_visible_ = true;

  bool virtual_key_state_[kVirtualKeyCount] = {false};

  ref_ptr<DisplayLink> display_link_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_PORT_LINUX_UI_X11_CONTROL_H_
