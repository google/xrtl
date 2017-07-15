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

#ifndef XRTL_PORT_WINDOWS_UI_WIN32_CONTROL_H_
#define XRTL_PORT_WINDOWS_UI_WIN32_CONTROL_H_

#include "xrtl/base/threading/event.h"
#include "xrtl/port/common/ui/timer_display_link.h"
#include "xrtl/port/windows/base/windows.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace ui {

// Win32 implementation of a control (child window).
class Win32Control : public Control {
 public:
  // Interface used by control containers.
  class ControlContainer {
   public:
    virtual void OnChildCreated(ref_ptr<Win32Control> child_control) {}
    virtual void OnChildDestroyed(ref_ptr<Win32Control> child_control) {}
  };

  Win32Control(ref_ptr<MessageLoop> message_loop, ControlContainer* container);
  ~Win32Control() override;

  HWND hwnd() const;
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

  void CheckMonitorChanged();
  Rect2D QueryBounds();

  static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT message, WPARAM w_param,
                                       LPARAM l_param);
  LRESULT WndProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
  bool HandleMouseMessage(UINT message, WPARAM w_param, LPARAM l_param);
  bool HandleKeyboardMessage(UINT message, WPARAM w_param, LPARAM l_param);

  void OnFocusChanged(bool is_focused);

  ControlContainer* container_ = nullptr;

  HDC dc_ = nullptr;
  HWND hwnd_ = nullptr;

  ref_ptr<Event> create_event_;
  ref_ptr<Event> destroy_event_;

  State state_ = State::kDestroyed;

  bool is_suspended_ = false;
  bool is_focused_ = true;
  Rect2D bounds_{{0, 0}, {128, 128}};
  gfx::rgba8_t background_color_;
  bool is_cursor_visible_ = true;

  // TODO(benvanik): switch to bitmap.
  uint8_t key_down_map_[256] = {0};

  ref_ptr<TimerDisplayLink> display_link_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_PORT_WINDOWS_UI_WIN32_CONTROL_H_
