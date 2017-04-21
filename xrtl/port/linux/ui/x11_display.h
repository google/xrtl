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

#ifndef XRTL_PORT_LINUX_UI_X11_DISPLAY_H_
#define XRTL_PORT_LINUX_UI_X11_DISPLAY_H_

#include <mutex>
#include <string>
#include <vector>

#include "xrtl/base/ref_ptr.h"
#include "xrtl/port/common/base/threading/epoll_message_loop.h"
#include "xrtl/port/linux/ui/x11_headers.h"

namespace xrtl {
namespace ui {

// X11 display connection wrapper.
// This pools connections by 'DISPLAY' placement, keeping them alive for at
// least the duration of use. It may keep connections alive even after they have
// been fully released for performance reasons.
class X11Display : public RefObject<X11Display> {
 public:
  class Listener {
   public:
    // Handles a single X11 event.
    // The event will be filtered based on the listener registration options.
    virtual bool OnXEvent(::XEvent* x_event) = 0;
  };

  // Connects to an X11 display.
  // If no placement is specified (empty string) the DISPLAY environment
  // variable will be used.
  // The reference must be held so long as X11 services are required.
  // Returns nullptr if the X server is not available.
  static ref_ptr<X11Display> Connect(std::string placement,
                                     ref_ptr<EpollMessageLoop> message_loop);

  ~X11Display();

  const std::string& placement() const { return placement_; }
  ref_ptr<EpollMessageLoop> message_loop() const { return message_loop_; }
  ::Display* display_handle() const { return display_handle_; }

  // Adds a listener using the given X11 ::Window handle as a filter.
  // The listener will begin receiving all events targeted at that window and
  // continue to receive them until RemoveWindowListener is called.
  // The listener must be kept alive for as long as it is registered.
  void AddWindowListener(Listener* listener, ::Window window_filter);

  // Removes a previously added listener. After this the listener will receive
  // no more events and may be deleted.
  void RemoveWindowListener(Listener* listener);

  static void Delete(X11Display* ptr);

 private:
  struct ListenerEntry {
    Listener* listener;
    ::Window window_filter;
  };

  X11Display(std::string placement, ref_ptr<EpollMessageLoop> message_loop);

  // Attempts to connect to the local X server.
  // Returns false if the X server is not available.
  bool TryConnect();
  void Disconnect();

  // Called whenever the fd is written. May be multiple events pending.
  void HandleEvents();

  std::string placement_;
  ref_ptr<EpollMessageLoop> message_loop_;

  ::Display* display_handle_ = nullptr;
  int fd_ = -1;

  std::mutex mutex_;
  std::vector<ListenerEntry> listeners_;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_PORT_LINUX_UI_X11_DISPLAY_H_
