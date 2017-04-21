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

#ifndef XRTL_PORT_LINUX_UI_X11_WINDOW_H_
#define XRTL_PORT_LINUX_UI_X11_WINDOW_H_

#include <mutex>
#include <string>

#include "xrtl/port/linux/ui/x11_control.h"
#include "xrtl/port/linux/ui/x11_headers.h"
#include "xrtl/ui/window.h"

namespace xrtl {
namespace ui {

// X11 implementation of a window.
class X11Window : public Window, private X11Control::ControlContainer {
 public:
  explicit X11Window(ref_ptr<MessageLoop> message_loop);
  ~X11Window() override;

  std::string title() override;
  void set_title(std::string title) override;

  ref_ptr<Control> root_control() override;

  ref_ptr<WaitHandle> Open() override;
  ref_ptr<WaitHandle> Close() override;

 protected:
  // From ControlContainer:
  void OnChildCreated(ref_ptr<X11Control> child_control) override;

  std::mutex mutex_;

  std::string title_;

  ref_ptr<X11Control> control_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_PORT_LINUX_UI_X11_WINDOW_H_
