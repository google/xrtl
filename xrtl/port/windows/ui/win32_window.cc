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

#include "xrtl/port/windows/ui/win32_window.h"

#include "xrtl/base/logging.h"

namespace xrtl {
namespace ui {

ref_ptr<Window> Window::Create(ref_ptr<MessageLoop> message_loop) {
  return make_ref<Win32Window>(message_loop);
}

Win32Window::Win32Window(ref_ptr<MessageLoop> message_loop)
    : Window(message_loop) {
  control_ = make_ref<Win32Control>(
      message_loop, static_cast<Win32Control::ControlContainer*>(this));
}

Win32Window::~Win32Window() { control_.reset(); }

std::string Win32Window::title() {
  std::lock_guard<std::mutex> lock(mutex_);
  return title_;
}

void Win32Window::set_title(std::string title) {
  std::lock_guard<std::mutex> lock(mutex_);
  title_ = title;
  if (control_ && control_->is_active()) {
    ::SetWindowTextA(control_->hwnd(), title_.c_str());
  }
}

ref_ptr<Control> Win32Window::root_control() { return control_.As<Control>(); }

ref_ptr<WaitHandle> Win32Window::Open() { return control_->Create(); }

ref_ptr<WaitHandle> Win32Window::Close() { return control_->Destroy(); }

void Win32Window::OnChildCreated(ref_ptr<Win32Control> child_control) {
  // Set values now that we have a window.
  set_title(title());
}

}  // namespace ui
}  // namespace xrtl
