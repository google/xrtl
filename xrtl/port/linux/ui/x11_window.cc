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

#include "xrtl/port/linux/ui/x11_window.h"

#include "xrtl/base/logging.h"

namespace xrtl {
namespace ui {

ref_ptr<Window> Window::Create(ref_ptr<MessageLoop> message_loop) {
  return make_ref<X11Window>(message_loop);
}

X11Window::X11Window(ref_ptr<MessageLoop> message_loop) : Window(message_loop) {
  control_ = make_ref<X11Control>(
      message_loop, static_cast<X11Control::ControlContainer*>(this));
}

X11Window::~X11Window() { control_.reset(); }

std::string X11Window::title() {
  std::lock_guard<std::mutex> lock(mutex_);
  return title_;
}

void X11Window::set_title(std::string title) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    title_ = title;
  }
  if (control_ && control_->is_active()) {
    ::XStoreName(control_->display_handle(), control_->window_handle(),
                 title.c_str());
    ::XFlush(control_->display_handle());
  }
}

ref_ptr<Control> X11Window::root_control() {
  return ref_ptr<Control>(control_.get());
}

ref_ptr<WaitHandle> X11Window::Open() { return control_->Create(); }

ref_ptr<WaitHandle> X11Window::Close() { return control_->Destroy(); }

void X11Window::OnChildCreated(ref_ptr<X11Control> child_control) {
  // Set values now that we have a window.
  set_title(title());
}

}  // namespace ui
}  // namespace xrtl
