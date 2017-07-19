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

#ifndef XRTL_UI_CONTROL_H_
#define XRTL_UI_CONTROL_H_

#include <memory>
#include <mutex>
#include <utility>

#include "xrtl/base/geometry.h"
#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/base/threading/message_loop.h"
#include "xrtl/gfx/color.h"
#include "xrtl/ui/input_events.h"

namespace xrtl {
namespace ui {

class DisplayLink;

// A user-visible system control that can be used as a swap chain target or
// presentation surface.
//
// Controls may be created and destroyed repeatedly. They will be created when
// parented to a platform window and destroyed when unparented in addition to
// spurious recreation caused by system events (such as graphics hardware reset
// or window manager changes).
//
// Only methods that specifically call out being thread-safe should be used from
// arbitrary threads. All other methods must only be called from the message
// loop the control is associated with.
//
// Usage:
//  auto control = Control::Create(message_loop);
//  control->set_listener(this);
//  ... configure control ...
//  parent_window->set_content_control(control);
class Control : public RefObject<Control> {
 public:
  // Opaque platform handle.
  using PlatformHandle = uintptr_t;

  // State of the control.
  enum class State {
    // Control is currently being created and will soon transition to the
    // kCreated state. It does not yet have a usable platform handle.
    kCreating,
    // Control is created and has a platform handle available for use.
    kCreated,

    // Control is currently being destroyed and will soon transition to
    // the kDestroyed state.
    kDestroying,
    // Control is destroyed.
    // It cannot be used as there is no corresponding platform handle.
    kDestroyed,
  };

  // Control event listener interface.
  // These methods can be overridden to handle control-specific events.
  //
  // All listener callbacks occur on the message loop associated with the
  // control.
  struct Listener {
    // Handles errors in the window system.
    // If this is received the control is likely broken and must be destroyed.
    virtual void OnError(ref_ptr<Control> target) {}

    // Handles the control beginning to be created.
    // It does not yet have a platform handle that is usable.
    virtual void OnCreating(ref_ptr<Control> target) {}
    // Handles the control ending creation.
    // It now has a usable platform handle.
    virtual void OnCreated(ref_ptr<Control> target) {}

    // Handles the control beginning to be destroyed.
    virtual void OnDestroying(ref_ptr<Control> target) {}
    // Handles the control ending destruction.
    // It no longer has a valid platform handle.
    virtual void OnDestroyed(ref_ptr<Control> target) {}

    // Handles the user changing the system theme (metrics/colors/etc).
    virtual void OnSystemThemeChanged(ref_ptr<Control> target) {}

    // Handles the control suspend state changing to the given new value.
    virtual void OnSuspendChanged(ref_ptr<Control> target, bool is_suspended) {}
    // Handles the control focus changing to the given new value.
    virtual void OnFocusChanged(ref_ptr<Control> target, bool is_focused) {}

    // Handles the control resizing to the given new bounds.
    virtual void OnResized(ref_ptr<Control> target, Rect2D bounds) {}
  };
  using ListenerPtr = std::unique_ptr<Listener, void (*)(Listener*)>;

  // Control input event listener interface.
  //
  // All listener callbacks occur on the message loop associated with the
  // control.
  struct InputListener {
    // Handles the start of a key press.
    virtual void OnKeyDown(ref_ptr<Control> target, const KeyboardEvent& ev) {}
    // Handles the end of a key press.
    virtual void OnKeyUp(ref_ptr<Control> target, const KeyboardEvent& ev) {}
    // Handles system key press events.
    virtual void OnKeyPress(ref_ptr<Control> target, const KeyboardEvent& ev) {}

    // Handles the start of a mouse button press.
    virtual void OnMouseDown(ref_ptr<Control> target, const MouseEvent& ev) {}
    // Handles the end of a mouse button press.
    virtual void OnMouseUp(ref_ptr<Control> target, const MouseEvent& ev) {}
    // Handles the mouse leaving the control.
    virtual void OnMouseOut(ref_ptr<Control> target, const MouseEvent& ev) {}
    // Handles mouse movement.
    virtual void OnMouseMove(ref_ptr<Control> target, const MouseEvent& ev) {}
    // Handles mouse wheel changes.
    virtual void OnMouseWheel(ref_ptr<Control> target, const MouseEvent& ev) {}
  };
  using InputListenerPtr =
      std::unique_ptr<InputListener, void (*)(InputListener*)>;

  // Creates a new control using the given message loop for event dispatch.
  static ref_ptr<Control> Create(ref_ptr<MessageLoop> message_loop);

  // TODO(benvanik): API for wrapping existing controls by platform handle.

  virtual ~Control() = default;

  // Message loop the control is using for event dispatch.
  ref_ptr<MessageLoop> message_loop() const { return message_loop_; }

  // Platform handle of the control (such as an HWND) if the control is created.
  // Implementation:
  //  Android: ANativeWindow
  //  iOS: UIView
  //  MacOS: ?
  //  Windows: HWND
  //  X11: ::Window
  virtual PlatformHandle platform_handle() = 0;

  // Platform display handle if the control is created.
  // This may be nullptr on systems that have no display handle concept.
  // Implementation:
  //  X11: ::Display
  virtual PlatformHandle platform_display_handle() = 0;

  // Sets a listener that will receive event notifications.
  // If the listener is passed as a naked pointer it must be kept alive for the
  // lifetime of the control.
  void set_listener(ListenerPtr listener);
  void set_listener(Listener* listener) {
    set_listener(ListenerPtr(listener, [](Listener*) {}));
  }

  // Sets a listener that will receive input notifications.
  // If the listener is passed as a naked pointer it must be kept alive for the
  // lifetime of the control.
  void set_input_listener(InputListenerPtr input_listener);
  void set_input_listener(InputListener* input_listener) {
    set_input_listener(InputListenerPtr(input_listener, [](InputListener*) {}));
  }

  // Returns the current control state.
  // Control active state is tied to their current parent or always inactive
  // if they are not parented.
  virtual State state() = 0;
  // Returns true if the control is 'active' (possibly user visible, etc).
  virtual bool is_active() = 0;

  // Whether the control is suspended.
  // Suspended control may still need to be painted (for the task switcher/etc).
  virtual bool is_suspended() = 0;
  virtual void set_suspended(bool suspended) = 0;

  // Whether the control has input focus.
  virtual bool is_focused() = 0;
  virtual void set_focused(bool focused) = 0;

  // Bounds of the control on the parent window, in pixels.
  // When the control represents a top-level system window this is the interior
  // of the window, excluding the title bar and other decorations. Thus,
  // positioning 0,0 will put the title bar offscreen.
  virtual Rect2D bounds() = 0;
  virtual void set_bounds(Rect2D bounds) = 0;

  // Size of the control on the parent window, in pixels.
  // Equivalent to `bounds().size`.
  Size2D size() { return bounds().size; }
  void set_size(Size2D size) {
    Rect2D current_bounds = bounds();
    current_bounds.size = size;
    set_bounds(current_bounds);
  }

  // Background color of the control if not completely filled when painted.
  // Setting this to a non-transparent color will generally increase
  // performance.
  virtual gfx::rgba8_t background_color() = 0;
  virtual void set_background_color(gfx::rgba8_t background_color) = 0;

  // Whether the system cursor is currently visible.
  // Prefer using a custom system cursor to drawing your own as the system
  // cursor is handled by the compositor and update regardless of framerate.
  virtual bool is_cursor_visible() = 0;
  virtual void set_cursor_visible(bool cursor_visible) = 0;

  // TODO(benvanik): custom cursor API.

  // TODO(benvanik): context menu API: ShowContextMenu(...), HideContextMenus().

  // Returns a display link for the control that can be used to synchronize
  // rendering. Each control in an application may have its own display link
  // based on the display it is presented on so it is best not to share them.
  virtual ref_ptr<DisplayLink> display_link() = 0;

  // Creates the control.
  // Once created controls must be destroyed with Destroy.
  // Returns a wait handle that will be signaled once the control has completed
  // creation. Is a no-op if called while the control is already created.
  // If the control cannot be opened an OnError callback will be made on the
  // listener.
  virtual ref_ptr<WaitHandle> Create() = 0;

  // Destroys the control.
  // Returns a wait handle that will be signaled once the control has completed
  // destruction. Is a no-op if called while the control is already destroyed.
  virtual ref_ptr<WaitHandle> Destroy() = 0;

  // Invalidates the control contents and requests a repaint.
  virtual void Invalidate() = 0;

 protected:
  explicit Control(ref_ptr<MessageLoop> message_loop)
      : message_loop_(std::move(message_loop)) {}

  void PostError();
  void PostCreating();
  void PostCreated();
  void PostDestroying();
  void PostDestroyed();
  void PostSystemThemeChanged();
  void PostSuspendChanged(bool is_suspended);
  void PostFocusChanged(bool is_focused);
  void PostResized(Rect2D bounds);

  void ResetEventShadows();
  void PostEvent(std::function<void(Listener*, ref_ptr<Control>)> callback);
  void PostInputEvent(
      std::function<void(InputListener*, ref_ptr<Control>)> callback);

  MessageLoop::TaskList pending_task_list_;
  ref_ptr<MessageLoop> message_loop_;

  std::recursive_mutex mutex_;

  std::mutex listener_mutex_;
  ListenerPtr listener_{nullptr, [](Listener*) {}};
  std::mutex input_listener_mutex_;
  InputListenerPtr input_listener_{nullptr, [](InputListener*) {}};

  bool has_posted_suspended_ = false;
  bool has_posted_focused_ = false;
  bool has_posted_bounds_ = false;
  bool is_suspended_shadow_ = false;
  bool is_focused_shadow_ = true;
  Rect2D bounds_shadow_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_CONTROL_H_
