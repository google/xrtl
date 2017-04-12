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

#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>

#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/message_loop.h"
#include "xrtl/base/threading/thread.h"

namespace xrtl {

// CFRunLoop message loop implementation.
class CFRunLoopMessageLoop : public MessageLoop {
 public:
  CFRunLoopMessageLoop();
  ~CFRunLoopMessageLoop() override;

  bool is_loop_thread() override { return thread_->is_current(); }

  void MarshalSync(std::function<void()> callback) override;

  ref_ptr<WaitHandle> Exit() override;

  uintptr_t native_handle() override { return thread_->native_handle(); }

 private:
  void ScheduleTask(ref_ptr<Task> task) override;
  void DescheduleTask(ref_ptr<Task> task) override;

  static void TimerCallback(CFRunLoopTimerRef timer, void* info);

  // Thread that the message loop runs on.
  ref_ptr<Thread> thread_;

  // The platform run loop we use for message dispatch.
  CFRunLoopRef run_loop_ = nullptr;
  // A dummy source that keeps the run loop alive.
  CFRunLoopSourceRef dummy_source_ = nullptr;
};

ref_ptr<MessageLoop> MessageLoop::Create() {
  return make_ref<CFRunLoopMessageLoop>();
}

CFRunLoopMessageLoop::CFRunLoopMessageLoop() {
  // We run a thread dedicated to the loop.
  Thread::CreateParams create_params;
  create_params.name = "CFRunLoopMessageLoop";
  ref_ptr<Event> ready_fence = Event::CreateFence();
  thread_ = Thread::Create(create_params, [this, &ready_fence]() {
    // Create the run loop (implicitly on first get).
    run_loop_ = CFRunLoopGetCurrent();
    DCHECK(run_loop_);
    CFRetain(run_loop_);

    // Create the dummy source that keeps the loop alive.
    CFRunLoopSourceContext source_context = {0};
    dummy_source_ =
        CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &source_context);
    CFRunLoopAddSource(run_loop_, dummy_source_, kCFRunLoopCommonModes);

    OnEnter();

    // Signal the ready fence which lets the ctor continue.
    ready_fence->Set();

    @autoreleasepool {
      // Blocking run loop that will only return when CFRunLoopStop is called.
      CFRunLoopRun();
    }

    // NOTE: when CFRunLoopRun() returns 'this' is no longer valid. Don't do
    //       anything here!
  });

  // Wait until our message loop is created and ready to receive messages.
  Thread::Wait(ready_fence);
  ready_fence.reset();
}

CFRunLoopMessageLoop::~CFRunLoopMessageLoop() {
  DCHECK(!is_loop_thread());

  // Kill the run loop so we get no more callbacks.
  if (run_loop_) {
    // TODO(benvanik): ensure dead? all invalidated?
    if (dummy_source_) {
      CFRunLoopRemoveSource(run_loop_, dummy_source_, kCFRunLoopCommonModes);
      CFRelease(dummy_source_);
      dummy_source_ = nullptr;
    }
    CFRelease(run_loop_);
    run_loop_ = nullptr;
  }
}

void CFRunLoopMessageLoop::MarshalSync(std::function<void()> callback) {
  if (is_loop_thread()) {
    // Can't marshal to ourselves; just run inline.
    callback();
    return;
  }

  // We can post with data directly on the stack because we know we'll be
  // waiting on the fence.
  __block auto callback_baton = std::move(callback);
  __block auto fence_event = Event::CreateFence();
  CFRunLoopPerformBlock(run_loop_, kCFRunLoopCommonModes, ^{
    @autoreleasepool {
      callback_baton();
      fence_event->Set();
      CFRunLoopWakeUp(run_loop_);
    }
  });
  CFRunLoopWakeUp(run_loop_);
  Thread::Wait(fence_event);
}

void CFRunLoopMessageLoop::ScheduleTask(ref_ptr<Task> task) {
  MessageLoop::ScheduleTask(task);

  // TODO(benvanik): fast path for immediate tasks. Needs to support cancel.

  // Setup the timer with our settings and queue it up.
  // We'll stash the handle on the task so we can cancel it later.
  CFRunLoopTimerContext timer_context = {0};
  timer_context.info = task.get();
  timer_context.retain = [](const void* info) {
    auto task = const_cast<Task*>(reinterpret_cast<const Task*>(info));
    task->AddReference();
    return info;
  };
  timer_context.release = [](const void* info) {
    auto task = const_cast<Task*>(reinterpret_cast<const Task*>(info));
    task->ReleaseReference();
  };
  CFRunLoopTimerRef timer = CFRunLoopTimerCreate(
      kCFAllocatorDefault,
      CFAbsoluteTimeGetCurrent() + task->delay_millis().count() / 1000.0f,
      task->period_millis().count() / 1000.0f, 0, 0, TimerCallback,
      &timer_context);

  // Retain timer and stash on task.
  CFRetain(timer);
  task->set_platform_handle(reinterpret_cast<uintptr_t>(timer));

  // Schedule for execution.
  CFRunLoopAddTimer(run_loop_, timer, kCFRunLoopDefaultMode);
  CFRelease(timer);
}

void CFRunLoopMessageLoop::TimerCallback(CFRunLoopTimerRef timer, void* info) {
  auto task = ref_ptr<Task>(reinterpret_cast<Task*>(info));
  @autoreleasepool {
    task->message_loop()->InvokeTask(task);
  }
}

void CFRunLoopMessageLoop::DescheduleTask(ref_ptr<Task> task) {
  // NOTE: we hold the Task lock.

  // Invalidate the timer. This will prevent it from executing.
  CFRunLoopTimerRef timer =
      reinterpret_cast<CFRunLoopTimerRef>(task->platform_handle());
  if (timer) {
    task->set_platform_handle(0);
    CFRunLoopTimerInvalidate(timer);

    // Release timer.
    CFRelease(timer);
  }

  MessageLoop::DescheduleTask(std::move(task));
}

ref_ptr<WaitHandle> CFRunLoopMessageLoop::Exit() {
  // Note that the CFRunLoopStop may cause us to exit and try to delete
  // ourselves immediately, so we need to make sure we stay alive with this
  // ref_ptr.
  ref_ptr<WaitHandle> wait_handle(this);

  // Stop the loop, if it is still running.
  MarshalSync([this]() {
    OnExit();
    CFRunLoopStop(run_loop_);
  });

  return wait_handle;
}

}  // namespace xrtl
