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

#include <utility>

namespace xrtl {
namespace ui {

namespace {

// A weak list of all active display connections.
std::recursive_mutex displays_mutex_;
std::vector<X11Display*> displays_;

int XErrorHandler(Display* display, ::XErrorEvent* event) {
  const size_t kBufSize = 256;
  char request_code_buf[kBufSize];
  ::XGetErrorDatabaseText(display, "XRequest",
                          std::to_string(event->request_code).c_str(), "",
                          request_code_buf, kBufSize);
  char error_code_buf[kBufSize];
  ::XGetErrorText(display, event->error_code, error_code_buf, kBufSize);
  LOG(FATAL) << "X error:"
             << " request_code=" << static_cast<int>(event->request_code)
             << " \"" << request_code_buf << "\""
             << " minor_code=" << static_cast<int>(event->minor_code)
             << " serial=" << static_cast<int>(event->serial)
             << " error_code=" << static_cast<int>(event->error_code) << " \""
             << error_code_buf << "\"";
  return 0;
}

}  // namespace

ref_ptr<X11Display> X11Display::Connect(
    std::string placement, ref_ptr<EpollMessageLoop> message_loop) {
  // Must always be called before we attempt to use X. Safe to call many times.
  ::XInitThreads();

  // Set an error handler so we log X11 errors properly.
  ::XSetErrorHandler(&XErrorHandler);

  // Try to find an existing display for the given loop.
  std::lock_guard<std::recursive_mutex> lock(displays_mutex_);
  for (X11Display* display : displays_) {
    if (display->placement() == placement &&
        display->message_loop() == message_loop) {
      return ref_ptr<X11Display>(display);
    }
  }

  // No existing display found; create a new one.
  ref_ptr<X11Display> display{new X11Display(placement, message_loop)};
  if (!display->TryConnect()) {
    LOG(ERROR) << "Could not create display";
    return {nullptr};
  }
  displays_.push_back(display.get());
  return display;
}

void X11Display::Delete(X11Display* ptr) {
  // We need to hold a lock on the display list while deleting. This prevents
  // anyone else from trying to steal our instance we are going to kill.
  std::lock_guard<std::recursive_mutex> lock(displays_mutex_);
  for (auto it = displays_.begin(); it != displays_.end(); ++it) {
    if (*it == ptr) {
      displays_.erase(it);
      break;
    }
  }

  // Disconnect and delete the display.
  delete ptr;
}

X11Display::X11Display(std::string placement,
                       ref_ptr<EpollMessageLoop> message_loop)
    : placement_(std::move(placement)),
      message_loop_(std::move(message_loop)) {}

X11Display::~X11Display() { Disconnect(); }

bool X11Display::TryConnect() {
  // Try to open the display.
  display_handle_ =
      ::XOpenDisplay(placement_.empty() ? nullptr : placement_.c_str());
  if (!display_handle_) {
    LOG(ERROR) << "Cannot connect to X server (XOpenDisplay failed)";
    return false;
  }

  // Get socket we can listen on.
  fd_ = ::XConnectionNumber(display_handle_);
  if (fd_ == -1) {
    LOG(ERROR) << "Unable to get display socket";
    return false;
  }

  // Register fd read callback so that we know when to process our events.
  message_loop_->RegisterReader(fd_, [this]() { HandleEvents(); });

  // Register a loop exit handler to clean up when the loop exits.
  // This will force the display to stay alive on the loop that created it.
  ref_ptr<X11Display> self(this);
  auto self_baton = MoveToLambda(self);
  message_loop_->RegisterExitCallback(
      [self_baton]() { self_baton.value->Disconnect(); });

  return true;
}

void X11Display::Disconnect() {
  if (fd_ != -1) {
    message_loop_->UnregisterReader(fd_);
    fd_ = -1;
  }

  if (display_handle_) {
    ::XSync(display_handle_, True);  // Discard all previous events.
    ::XCloseDisplay(display_handle_);
    display_handle_ = nullptr;
  }
}

void X11Display::AddWindowListener(Listener* listener, ::Window window_filter) {
  std::lock_guard<std::mutex> lock(mutex_);
  listeners_.push_back({listener, window_filter});
}

void X11Display::RemoveWindowListener(Listener* listener) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto it = listeners_.begin(); it != listeners_.end(); ++it) {
    if (it->listener == listener) {
      listeners_.erase(it);
      break;
    }
  }
}

void X11Display::HandleEvents() {
  // Spin so long as there are events pending in the local client queue.
  while (::XPending(display_handle_) > 0) {
    // Pop the next event from the queue.
    ::XEvent x_event;
    ::XNextEvent(display_handle_, &x_event);

    // Determine the target window. If the type has no window we route to all
    // listeners.
    ::Window target_window = 0;
    switch (x_event.type) {
      case KeyPress:
      case KeyRelease:
        target_window = x_event.xkey.window;
        break;
      case ButtonPress:
      case ButtonRelease:
        target_window = x_event.xbutton.window;
        break;
      case MotionNotify:
        target_window = x_event.xmotion.window;
        break;
      case EnterNotify:
      case LeaveNotify:
        target_window = x_event.xcrossing.window;
        break;
      case FocusIn:
      case FocusOut:
        target_window = x_event.xfocus.window;
        break;
      case KeymapNotify:
        target_window = x_event.xkeymap.window;
        break;
      case Expose:
        target_window = x_event.xexpose.window;
        break;
      case GraphicsExpose:
        // TODO(benvanik): see if drawable is valid?
        // target_window = x_event.xgraphicsexpose.window;
        break;
      case NoExpose:
        // TODO(benvanik): see if drawable is valid?
        // target_window = x_event.xnoexpose.window;
        break;
      case VisibilityNotify:
        target_window = x_event.xvisibility.window;
        break;
      case CreateNotify:
        target_window = x_event.xcreatewindow.window;
        break;
      case DestroyNotify:
        target_window = x_event.xdestroywindow.window;
        break;
      case UnmapNotify:
        target_window = x_event.xunmap.window;
        break;
      case MapNotify:
        target_window = x_event.xmap.window;
        break;
      case MapRequest:
        target_window = x_event.xmaprequest.window;
        break;
      case ReparentNotify:
        target_window = x_event.xreparent.window;
        break;
      case ConfigureNotify:
        target_window = x_event.xconfigure.window;
        break;
      case ConfigureRequest:
        target_window = x_event.xconfigurerequest.window;
        break;
      case GravityNotify:
        target_window = x_event.xgravity.window;
        break;
      case ResizeRequest:
        target_window = x_event.xresizerequest.window;
        break;
      case CirculateNotify:
        target_window = x_event.xcirculate.window;
        break;
      case CirculateRequest:
        target_window = x_event.xcirculaterequest.window;
        break;
      case PropertyNotify:
        target_window = x_event.xproperty.window;
        break;
      case SelectionClear:
        target_window = x_event.xselectionclear.window;
        break;
      case SelectionRequest:
        // TODO(benvanik): verify this should be requestor and not owner.
        target_window = x_event.xselectionrequest.requestor;
        break;
      case SelectionNotify:
        target_window = x_event.xselection.requestor;
        break;
      case ColormapNotify:
        target_window = x_event.xcolormap.window;
        break;
      case ClientMessage:
        target_window = x_event.xclient.window;
        break;
      case MappingNotify:
        target_window = x_event.xmapping.window;
        break;
      default:
      case GenericEvent:
        // Broadcast to all.
        break;
    }

    // Dispatch to listeners that match the filter.
    // TODO(benvanik): ensure safe to unregister listener from within call.
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (const auto& listener_entry : listeners_) {
        if (!target_window || listener_entry.window_filter == target_window) {
          if (listener_entry.listener->OnXEvent(&x_event)) {
            break;
          }
        }
      }
    }
  }
}

}  // namespace ui
}  // namespace xrtl
