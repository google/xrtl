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

#ifndef XRTL_BASE_THREADING_EVENT_H_
#define XRTL_BASE_THREADING_EVENT_H_

#include "xrtl/base/threading/wait_handle.h"

namespace xrtl {

// An event based on the Win32 event object.
// This is effectively a binary semaphore with a maximum_count of 1 when running
// in auto-reset mode.
//
// Usage, as a fence:
//  // This allows other threads to block until the event is set.
//  // Once set, all threads will wake.
//  auto fence_event = Event::CreateManualResetEvent(false);
//  RunOnOtherThread1([&]() { Thread::Wait(fence_event); });
//  RunOnOtherThread2([&]() { Thread::Wait(fence_event); });
//  fence_event->Set();
//
// Usage, as a pulse event (binary semaphore):
//  auto pulse_event = Event::CreateAutoResetEvent(false);
//  RunOnOtherThread([&]() {
//    while (true) {
//      Thread::Wait(pulse_event);  // Wait for work.
//      // ... do work.
//    }
//  });
//  // ... queue work.
//  pulse_event->Set();
//
// Reference:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682396(v=vs.85).aspx
class Event : public WaitHandle {
 public:
  // Creates a fence event object.
  // This is a manual reset event with an initial state of false but is a bit
  // more readable when the intent is for a set-once fence.
  //
  // Usage:
  //  auto fence_event = Event::CreateFence();
  //  RunOnOtherThread1([&]() { Thread::Wait(fence_event); });
  //  RunOnOtherThread2([&]() { Thread::Wait(fence_event); });
  //  fence_event->Set();
  static ref_ptr<Event> CreateFence() { return CreateManualResetEvent(false); }

  // Creates a new manual reset event object with the initial state.
  // While signaled all waits will pass and all threads already waiting on the
  // event will be released.
  // Use the Reset function to set the event to nonsignaled.
  // If initial_state is true the event will start in the signaled state.
  static ref_ptr<Event> CreateManualResetEvent(bool initial_state);

  // Creates a new auto reset event object with the initial state.
  // The event will automatically reset to nonsignaled after a single waiting
  // thread has been released, and if no threads are waiting the first wait
  // attempt will pass before the event is reset. If multiple threads are
  // waiting the one chosen to wake is undefined.
  // If initial_state is true the event will start in the signaled state.
  static ref_ptr<Event> CreateAutoResetEvent(bool initial_state);

  // Sets the specified event object to the signaled state.
  // If this is a manual reset event the event stays signaled until Reset is
  // called. If this is an auto reset event it will remain signaled until
  // exactly one wait is satisfied.
  virtual void Set() = 0;

  // Resets the specified event object to the nonsignaled state.
  // Resetting an event that is already reset has no effect.
  virtual void Reset() = 0;
};

}  // namespace xrtl

#endif  // XRTL_BASE_THREADING_EVENT_H_
