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

#ifndef XRTL_UI_WINDOW_H_
#define XRTL_UI_WINDOW_H_

#include <memory>
#include <string>
#include <utility>

#include "xrtl/base/macros.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/base/threading/message_loop.h"
#include "xrtl/ui/control.h"

namespace xrtl {
namespace ui {

// A top-level user-visible platform window.
// Windows wrap content controls and expose them to the platform.
// Most app logic should focus on the Control itself as most events will be
// routed to their listeners as well (such as visibility, sizing, etc). This
// also ensures that most of the app logic works when hosted within other
// controls or windows that the app does not have control over.
//
// Windows may be opened and closed repeatedly. Once opened with Open a window
// will remain open until explicitly closed with Close. Both window opening and
// closing are asynchronous operations and may take some time to complete. Use
// the wait handles returned by Open and Close to synchronize, if needed.
//
// Usage:
//  auto message_loop = MessageLoop::Create();
//  auto window = Window::Create(message_loop);
//  window->set_title("Hello!");
//  auto control = window->root_control();
//  control->set_listener(this);
//  window->Open();
//  OnSomeEvent() {
//    window->Close();
//  }
class Window : public RefObject<Window> {
 public:
  // Creates a new window using the given message loop for event dispatch.
  // The window will need to be configured by the caller and opened with
  // Open. The window remain open (even if set hidden) until Close is
  // called.
  static ref_ptr<Window> Create(ref_ptr<MessageLoop> message_loop);

  // TODO(benvanik): API for wrapping existing windows by platform handle.

  virtual ~Window() = default;

  // Message loop the window is using for event dispatch.
  ref_ptr<MessageLoop> message_loop() const { return message_loop_; }

  // Title displayed on the window.
  virtual std::string title() = 0;
  virtual void set_title(std::string title) = 0;

  // TODO(benvanik): custom icon API (need png buffer).

  // TODO(benvanik): main menu API.

  // The primary content control of the window.
  // The control will be automatically resized to fit the window as the window
  // size changes.
  virtual ref_ptr<Control> root_control() = 0;

  // Opens the window.
  // Once opened windows must be closed with Close.
  // Returns a wait handle that will be signaled once the window has completed
  // opening. Is a no-op if called while the window is already opened.
  // If the window cannot be opened an OnError callback will be made on the
  // listener.
  virtual ref_ptr<WaitHandle> Open() = 0;

  // Closes the window.
  // Returns a wait handle that will be signaled once the window has completed
  // closing. Is a no-op if called while the window is already closed.
  virtual ref_ptr<WaitHandle> Close() = 0;

 protected:
  explicit Window(ref_ptr<MessageLoop> message_loop)
      : message_loop_(std::move(message_loop)) {}

  MessageLoop::TaskList pending_task_list_;
  ref_ptr<MessageLoop> message_loop_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_WINDOW_H_
