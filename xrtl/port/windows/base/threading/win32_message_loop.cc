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

#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/message_loop.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

namespace {

// Custom window messages used by us for internal signaling:
const DWORD kMsgLoopMarshalSync = WM_APP + 0x100;
const DWORD kMsgLoopInvoke = WM_APP + 0x101;
const DWORD kMsgLoopQuit = WM_APP + 0x102;

// Temporary data used when performing a MarshalSync.
struct MarshalCall {
  ref_ptr<Event> fence;
  std::function<void()> callback;
};

}  // namespace

// Win32 message loop implementation.
class Win32MessageLoop : public MessageLoop {
 public:
  Win32MessageLoop();
  ~Win32MessageLoop() override;

  bool is_loop_thread() override { return thread_->is_current(); }

  void MarshalSync(std::function<void()> callback) override;

  ref_ptr<WaitHandle> Exit() override;

  uintptr_t native_handle() override { return thread_->native_handle(); }

 private:
  void ScheduleTask(ref_ptr<Task> task) override;
  void DescheduleTask(ref_ptr<Task> task) override;

  static void TimerQueueCallback(void* context, uint8_t);

  // Thread that the message loop runs on.
  ref_ptr<Thread> thread_;
  // Timer queue used for delayed tasks.
  HANDLE timer_queue_ = INVALID_HANDLE_VALUE;

  // All tasks that have been canceled since the last message loop pump.
  // Since we can't snoop the message loop and remove the tasks this is the
  // only safe way to ensure we keep the task objects alive.
  IntrusiveList<ref_ptr<Task>, offsetof(Task, loop_list_link_)> canceled_tasks_;
};

ref_ptr<MessageLoop> MessageLoop::Create() {
  return make_ref<Win32MessageLoop>();
}

Win32MessageLoop::Win32MessageLoop() {
  // Timer queue will process all our waits and marshal back on to the message
  // loop thread when it needs to invoke things.
  timer_queue_ = ::CreateTimerQueue();
  DCHECK_NE(timer_queue_, INVALID_HANDLE_VALUE);

  // We run a thread dedicated to the loop.
  Thread::CreateParams create_params;
  create_params.name = "Win32MessageLoop";
  ref_ptr<Event> ready_fence = Event::CreateFence();
  thread_ = Thread::Create(create_params, [this, &ready_fence]() {
    // Make a Win32 call to enable the thread queue. Until we do this posted
    // messages will be dropped on the floor.
    MSG msg;
    PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    OnEnter();

    // Signal the ready fence which lets the ctor continue.
    ready_fence->Set();

    // Sit and loop until we get a quit message.
    while (true) {
      // First peek to see if there are any messages waiting.
      // This lets us prevent blocking if we don't need to.
      if (!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD)) {
        // No messages were available.
        // We are going to block on GetMessage, so first run cleanup.
        {
          std::lock_guard<std::recursive_mutex> lock(task_mutex_);
          canceled_tasks_.clear();
        }

        // Now block until we get a message. Note that in a race where
        // PeekMessage fails this may pass immediately if another thread
        // inserted a message.
        if (!GetMessage(&msg, nullptr, 0, 0)) {
          OnExit();
          return;
        }
      }

      // Normal message handling for Windows messages.
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.wParam != reinterpret_cast<WPARAM>(this)) {
        continue;
      }

      // Handle our own messages.
      // We retain ourselves while handling the message as the callback may
      // delete the loop.
      ref_ptr<Win32MessageLoop> self_loop(this);
      switch (msg.message) {
        case kMsgLoopMarshalSync: {
          // Invoke the function on the loop thread and signal completion.
          auto marshal_call = reinterpret_cast<MarshalCall*>(msg.lParam);
          marshal_call->callback();
          marshal_call->callback = nullptr;
          marshal_call->fence->Set();
          break;
        }
        case kMsgLoopInvoke: {
          // Invoke the task on the loop thread.
          InvokeTask(ref_ptr<Task>(reinterpret_cast<Task*>(msg.lParam)));
          break;
        }
        case kMsgLoopQuit: {
          // Exit the while and end the thread.
          OnExit();
          return;
        }
      }
    }
  });

  // Wait until our message loop is created and ready to receive messages.
  Thread::Wait(ready_fence);
  ready_fence.reset();
}

Win32MessageLoop::~Win32MessageLoop() {
  DCHECK(!is_loop_thread());

  // Kill the timer queue so we get no more callbacks.
  if (timer_queue_ != INVALID_HANDLE_VALUE) {
    ::DeleteTimerQueueEx(timer_queue_, INVALID_HANDLE_VALUE);
    timer_queue_ = INVALID_HANDLE_VALUE;
  }
}

void Win32MessageLoop::MarshalSync(std::function<void()> callback) {
  if (is_loop_thread()) {
    // Can't marshal to ourselves; just run inline.
    callback();
    return;
  }

  // We can post with data directly on the stack because we know we'll be
  // waiting on the fence.
  MarshalCall marshal_call;
  marshal_call.fence = Event::CreateFence();
  marshal_call.callback = std::move(callback);

  // Post the request to the loop.
  ::PostThreadMessage(thread_->thread_id(), kMsgLoopMarshalSync,
                      reinterpret_cast<WPARAM>(this),
                      reinterpret_cast<LPARAM>(&marshal_call));

  // Wait for the request to complete.
  Thread::Wait(marshal_call.fence);
}

void Win32MessageLoop::ScheduleTask(ref_ptr<Task> task) {
  MessageLoop::ScheduleTask(task);

  DCHECK_NE(timer_queue_, INVALID_HANDLE_VALUE);

  // Fast path for immediate tasks. We just post the message.
  if (!task->delay_millis().count() && !task->period_millis().count()) {
    task->set_platform_handle(
        reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE));
    ::PostThreadMessage(thread_->thread_id(), kMsgLoopInvoke,
                        reinterpret_cast<WPARAM>(this),
                        reinterpret_cast<LPARAM>(task.get()));
    return;
  }

  // Setup the timer with our settings and queue it up.
  // We'll stash the handle on the task so we can cancel it later.
  HANDLE timer_handle = INVALID_HANDLE_VALUE;
  ULONG timer_flags = WT_EXECUTEINTIMERTHREAD;
  if (!task->period_millis().count()) {
    timer_flags |= WT_EXECUTEONLYONCE;
  }
  ::CreateTimerQueueTimer(
      &timer_handle, timer_queue_,
      static_cast<WAITORTIMERCALLBACK>(TimerQueueCallback), task.get(),
      static_cast<DWORD>(task->delay_millis().count()),
      static_cast<DWORD>(task->period_millis().count()), timer_flags);
  task->set_platform_handle(reinterpret_cast<uintptr_t>(timer_handle));
}

void Win32MessageLoop::DescheduleTask(ref_ptr<Task> task) {
  // NOTE: we hold the Task lock.

  HANDLE timer_handle = reinterpret_cast<HANDLE>(task->platform_handle());
  if (timer_handle != INVALID_HANDLE_VALUE) {
    task->set_platform_handle(
        reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE));
    ::DeleteTimerQueueTimer(timer_queue_, timer_handle, nullptr);
  }

  MessageLoop::DescheduleTask(task);

  // Move the task to the pending cancel list.
  // So long as the loop is still running we don't know when it will trigger
  // and we don't want it to use-after-free.
  {
    std::lock_guard<std::recursive_mutex> lock(task_mutex_);
    DCHECK(!pending_tasks_.contains(task));
    DCHECK(!canceled_tasks_.contains(task));
    canceled_tasks_.push_back(task);
  }
}

void Win32MessageLoop::TimerQueueCallback(void* context, uint8_t) {
  auto task = ref_ptr<Task>(reinterpret_cast<Task*>(context));

  // TODO(benvanik): check if alive before queuing?

  // This callback is just telling us the task is ready to run.
  // We're executing within the timer queue thread, *not* the loop thread, so
  // we need to marshal onto it.
  Win32MessageLoop* message_loop =
      static_cast<Win32MessageLoop*>(task->message_loop());
  ::PostThreadMessage(message_loop->thread_->thread_id(), kMsgLoopInvoke,
                      reinterpret_cast<WPARAM>(task->message_loop()),
                      reinterpret_cast<LPARAM>(task.get()));
}

ref_ptr<WaitHandle> Win32MessageLoop::Exit() {
  // Note that the PostThreadMessage may cause us to exit and try to delete
  // ourselves immediately, so we need to make sure we stay alive with this
  // ref_ptr.
  ref_ptr<WaitHandle> wait_handle(this);

  // Post a quit message to the loop. When it receives this it will exit.
  if (!Thread::TryWait(thread_)) {
    ::PostThreadMessage(thread_->thread_id(), kMsgLoopQuit,
                        reinterpret_cast<WPARAM>(this), 0);
  }

  return wait_handle;
}

}  // namespace xrtl
