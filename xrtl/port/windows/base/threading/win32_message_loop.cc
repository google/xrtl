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

#include "absl/base/call_once.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/message_loop.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

namespace {

const wchar_t* kMessageWindowClassName = L"XrtlMessageWindowClass";

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
  static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT message, WPARAM w_param,
                                       LPARAM l_param);
  LRESULT WndProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);

  void ScheduleTask(ref_ptr<Task> task) override;
  void DescheduleTask(ref_ptr<Task> task) override;

  static void TimerQueueCallback(void* context, uint8_t);

  // Thread that the message loop runs on.
  ref_ptr<Thread> thread_;
  // Hidden message window.
  HWND message_hwnd_ = nullptr;
  // Timer queue used for delayed tasks.
  HANDLE timer_queue_ = INVALID_HANDLE_VALUE;

  // Messages registered with the system we send to the window.
  UINT marshal_sync_message_ = 0;
  UINT invoke_message_ = 0;
  UINT quit_message_ = 0;

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

  // Ensure we create the window class we use for the hidden message window.
  // This should be process-local so we only need to do it once.
  static absl::once_flag register_class_flag;
  absl::call_once(register_class_flag, []() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = Win32MessageLoop::WndProcThunk;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = ::GetModuleHandle(nullptr);
    wcex.hIcon = nullptr;
    wcex.hIconSm = nullptr;
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = kMessageWindowClassName;
    if (!::RegisterClassExW(&wcex)) {
      LOG(FATAL) << "Unable to register window class";
    }
  });

  // Reserve message IDs unique to our app.
  marshal_sync_message_ =
      ::RegisterWindowMessageA("XRTL_MESSAGE_LOOP_MARSHAL_SYNC");
  invoke_message_ = ::RegisterWindowMessageA("XRTL_MESSAGE_LOOP_INVOKE");
  quit_message_ = ::RegisterWindowMessageA("XRTL_MESSAGE_LOOP_QUIT");

  // We run a thread dedicated to the loop.
  Thread::CreateParams create_params;
  create_params.name = "Win32MessageLoop";
  ref_ptr<Event> ready_fence = Event::CreateFence();
  thread_ = Thread::Create(create_params, [this, &ready_fence]() {
    // Create hidden message window.
    message_hwnd_ = ::CreateWindowExW(
        0, kMessageWindowClassName, L"(xrtl message loop)", 0, 0, 0, 0, 0,
        HWND_MESSAGE, nullptr, ::GetModuleHandle(nullptr), this);
    DCHECK(message_hwnd_);

    OnEnter();

    // Signal the ready fence which lets the ctor continue.
    ready_fence->Set();

    // Sit and loop until we get a quit message.
    while (true) {
      // First peek to see if there are any messages waiting.
      // This lets us prevent blocking if we don't need to.
      MSG msg;
      if (!::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD)) {
        // No messages were available.
        // We are going to block on GetMessage, so first run cleanup.
        {
          std::lock_guard<std::recursive_mutex> lock(task_mutex_);
          canceled_tasks_.clear();
        }

        // Now block until we get a message. Note that in a race where
        // PeekMessage fails this may pass immediately if another thread
        // inserted a message.
        if (!::GetMessage(&msg, nullptr, 0, 0)) {
          OnExit();
          return;
        }
      }

      // Normal message handling for Windows messages.
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);

      if (msg.message == quit_message_) {
        break;
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

  // Kill the message window.
  if (message_hwnd_) {
    ::DestroyWindow(message_hwnd_);
    message_hwnd_ = nullptr;
  }
}

LRESULT CALLBACK Win32MessageLoop::WndProcThunk(HWND hwnd, UINT message,
                                                WPARAM w_param,
                                                LPARAM l_param) {
  // Retrieve the target loop from the hwnd.
  Win32MessageLoop* message_loop = nullptr;
  if (message == WM_NCCREATE) {
    // The window has been created with the system.
    // This is called *inline* in the ::CreateWindow call, so we have to be
    // very careful what state we access.
    auto create_struct = reinterpret_cast<LPCREATESTRUCT>(l_param);
    message_loop =
        reinterpret_cast<Win32MessageLoop*>(create_struct->lpCreateParams);
    message_loop->message_hwnd_ = hwnd;
    ::SetWindowLongPtr(
        hwnd, GWLP_USERDATA,
        static_cast<__int3264>(reinterpret_cast<LONG_PTR>(message_loop)));
  } else {
    message_loop = reinterpret_cast<Win32MessageLoop*>(
        ::GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }
  if (message_loop) {
    return message_loop->WndProc(hwnd, message, w_param, l_param);
  } else {
    return ::DefWindowProc(hwnd, message, w_param, l_param);
  }
}

LRESULT Win32MessageLoop::WndProc(HWND hwnd, UINT message, WPARAM w_param,
                                  LPARAM l_param) {
  // Handle our messages to the hidden window.
  // We retain ourselves while handling the message as the callback may
  // delete the loop.
  if (message == marshal_sync_message_) {
    // Invoke the function on the loop thread and signal completion.
    ref_ptr<Win32MessageLoop> self_loop(this);
    auto marshal_call = reinterpret_cast<MarshalCall*>(l_param);
    marshal_call->callback();
    marshal_call->callback = nullptr;
    marshal_call->fence->Set();
    return 0;
  } else if (message == invoke_message_) {
    // Invoke the task on the loop thread.
    ref_ptr<Win32MessageLoop> self_loop(this);
    InvokeTask(ref_ptr<Task>(reinterpret_cast<Task*>(l_param)));
    return 0;
  } else if (message == quit_message_) {
    // Exit the while and end the thread.
    ref_ptr<Win32MessageLoop> self_loop(this);
    OnExit();
    return 0;
  } else {
    return ::DefWindowProc(hwnd, message, w_param, l_param);
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
  ::PostMessage(message_hwnd_, marshal_sync_message_,
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
    ::PostMessage(message_hwnd_, invoke_message_,
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
  ::PostMessage(message_loop->message_hwnd_, message_loop->invoke_message_,
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
    ::PostMessage(message_hwnd_, quit_message_, reinterpret_cast<WPARAM>(this),
                  0);
  }

  return wait_handle;
}

}  // namespace xrtl
