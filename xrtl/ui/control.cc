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

#include "xrtl/ui/control.h"

namespace xrtl {
namespace ui {

void Control::set_listener(ListenerPtr listener) {
  std::lock_guard<std::mutex> lock(listener_mutex_);
  listener_ = std::move(listener);
}

void Control::set_input_listener(InputListenerPtr input_listener) {
  std::lock_guard<std::mutex> lock(input_listener_mutex_);
  input_listener_ = std::move(input_listener);
}

void Control::PostError() {
  PostEvent([](Listener* listener, ref_ptr<Control> control) {
    listener->OnError(control);
  });
}

void Control::PostCreating() {
  PostEvent([](Listener* listener, ref_ptr<Control> control) {
    listener->OnCreating(control);
  });
}

void Control::PostCreated() {
  PostEvent([](Listener* listener, ref_ptr<Control> control) {
    listener->OnCreated(control);
  });
}

void Control::PostDestroying() {
  PostEvent([](Listener* listener, ref_ptr<Control> control) {
    listener->OnDestroying(control);
  });
}

void Control::PostDestroyed() {
  PostEvent([](Listener* listener, ref_ptr<Control> control) {
    listener->OnDestroyed(control);
  });
}

void Control::PostSystemThemeChanged() {
  PostEvent([](Listener* listener, ref_ptr<Control> control) {
    listener->OnSystemThemeChanged(control);
  });
}

void Control::PostSuspendChanged(bool is_suspended) {
  PostEvent([this, is_suspended](Listener* listener, ref_ptr<Control> control) {
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      if (has_posted_suspended_ && is_suspended == is_suspended_shadow_) {
        return;  // Debounce.
      }
      has_posted_suspended_ = true;
      is_suspended_shadow_ = is_suspended;
    }
    listener->OnSuspendChanged(control, is_suspended);
  });
}

void Control::PostFocusChanged(bool is_focused) {
  PostEvent([this, is_focused](Listener* listener, ref_ptr<Control> control) {
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      if (has_posted_focused_ && is_focused == is_focused_shadow_) {
        return;  // Debounce.
      }
      has_posted_focused_ = true;
      is_focused_shadow_ = is_focused;
    }
    listener->OnFocusChanged(control, is_focused);
  });
}

void Control::PostResized(Rect2D bounds) {
  PostEvent([this, bounds](Listener* listener, ref_ptr<Control> control) {
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      if (has_posted_bounds_ && bounds == bounds_shadow_) {
        return;  // Debounce.
      }
      has_posted_bounds_ = true;
      bounds_shadow_ = bounds;
    }
    listener->OnResized(control, bounds);
  });
}

void Control::ResetEventShadows() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  has_posted_suspended_ = false;
  has_posted_focused_ = false;
  has_posted_bounds_ = false;
}

void Control::PostEvent(
    std::function<void(Listener*, ref_ptr<Control>)> callback) {
  auto callback_baton = MoveToLambda(callback);
  message_loop_->MarshalSync([this, callback_baton]() {
    std::lock_guard<std::mutex> lock(listener_mutex_);
    if (listener_) {
      ref_ptr<Control> control{this};
      callback_baton.value(listener_.get(), control);
    }
  });
}

void Control::PostInputEvent(
    std::function<void(InputListener*, ref_ptr<Control>)> callback) {
  switch (state()) {
    case State::kCreating:
    case State::kDestroying:
    case State::kDestroyed:
      // Ignore input events when the control is not active.
      return;
    default:
      break;
  }
  auto callback_baton = MoveToLambda(callback);
  message_loop_->MarshalSync([this, callback_baton]() {
    std::lock_guard<std::mutex> lock(input_listener_mutex_);
    if (input_listener_) {
      ref_ptr<Control> control{this};
      callback_baton.value(input_listener_.get(), control);
    }
  });
}

}  // namespace ui
}  // namespace xrtl
