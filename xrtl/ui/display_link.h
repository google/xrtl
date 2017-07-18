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

#ifndef XRTL_UI_DISPLAY_LINK_H_
#define XRTL_UI_DISPLAY_LINK_H_

#include <chrono>
#include <functional>
#include <utility>

#include "xrtl/base/ref_ptr.h"

namespace xrtl {
namespace ui {

// Provides display refresh and synchronization support.
// To render into a control applications should request its display link and
// listen for refresh callbacks.
//
// Display links may be automatically paused by the target control such as when
// the control is minimized/suspended by the system. In these cases the display
// link will be resumed by the control automatically when possible unless the
// application explicitly calls Stop.
//
// Though the display link methods may be called from any thread the callback
// will only be issued on the message loop of the control the display link is
// attached to.
//
// DisplayLink roughly maps to the following platform concepts:
// - Android: Choreographer
// - iOS: CADisplayLink
// - MacOS: CVDisplayLink
// The fallback for other platforms is a nasty timer. Beware.
class DisplayLink : public RefObject<DisplayLink> {
 public:
  // A value that can be used to specify the preferred frames per second should
  // be kept as fast as possible.
  static constexpr int kLowLatency = 0;
  // A value that can be used to specify the preferred frames per second should
  // always match the maximum.
  static constexpr int kMaxDisplayRate = -1;

  virtual ~DisplayLink() = default;

  // True if the callbacks from this DisplayLink are accurate.
  // Some implementations are unable to provide high resolution timing or direct
  // system vsync listening. If that is the case it's recommended to instead
  // use dedicated render threads and blocking on swap chain presents.
  virtual bool is_accurate() = 0;

  // The maximum number of frames/second that the display can support.
  // For example, 60. This may change during execution if the parent control
  // is moved to other displays.
  virtual int max_frames_per_second() = 0;

  // The preferred frames per second the display is refreshing at.
  // The display will attempt to call back at this rate.
  virtual int preferred_frames_per_second() = 0;

  // Starts the display link refresh callback.
  // After calling this function refresh callbacks will start unless there are
  // suspend requests. Calling this method multiple times will reconfigure the
  // display link to use the new callback and preferred rate.
  //
  // The callback will receive a timestamp in microseconds UTC (similar to
  // SystemClock::now_utc_micros()). Always prefer to use this value to querying
  // the time yourself.
  //
  // The preferred frames per second value can be used to set the display link
  // to a lower refresh rate than it natively supports. Implementations may
  // adjust this value to ensure smooth frame rates (for example, passing
  // preferred = 35 when max = 60 may cause the implementation to round to 30).
  // If the preferred rate is omitted the maximum rate will be used.
  //
  // To unrestrict the callback rate and drive the link as fast as possible use
  // the kLowLatency constant for the preferred frame rate. The kMaxDisplayRate
  // constant can be used to allow the link to adjust its rate based on the
  // current display of the control.
  //
  // Callbacks will be executed on an arbitrary thread depending on
  // implementation. This may mean the calling thread (if it has a MessageLoop)
  // or others. Always ensure the callback either marshals to an appropriate
  // thread or ensures resources are guarded.
  //
  // This is safe to call from any thread.
  virtual void Start(std::function<void(std::chrono::microseconds)> callback,
                     int preferred_frames_per_second) = 0;
  void Start(std::function<void(std::chrono::microseconds)> callback) {
    Start(std::move(callback), kMaxDisplayRate);
  }

  // Stops the display link refresh callbacks.
  // After calling this function no more callbacks will be made until the
  // display link is restarted. Safe to call even if the timer has not been
  // started.
  //
  // This is safe to call from any thread. If a refresh callback is currently
  // executing on another thread this function will block until it completes.
  virtual void Stop() = 0;

  // Increments the suspend count of the display link and possibly suspends it.
  // If the display link has been started it will be paused until the suspend
  // count returns to 0.
  //
  // This is safe to call from any thread. If a refresh callback is currently
  // executing on another thread this function will block until it completes.
  virtual void Suspend() = 0;

  // Decrements the suspend count of the display link and possibly resumes it.
  // If the display link was not originally started this does nothing.
  //
  // This is safe to call from any thread.
  virtual void Resume() = 0;

 protected:
  DisplayLink() = default;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_DISPLAY_LINK_H_
