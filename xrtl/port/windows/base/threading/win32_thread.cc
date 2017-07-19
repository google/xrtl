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

#include <mutex>
#include <thread>

#include "xrtl/base/logging.h"
#include "xrtl/base/stopwatch.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/port/windows/base/threading/win32_wait_handle.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

namespace {

// Period of the global system clock we request in high-resolution timing mode.
// Timers and sleeps should get close to this level of accuracy when enabled.
constexpr uint32_t kHighResolutionTimingPeriodMillis = 1;

// TLS slot that holds the current Thread* value.
// The slot owns a reference so that the Thread stays alive so long as the value
// is set. When the thread exits the reference is cleared up automatically.
DWORD current_thread_fls_index_ = -1;

class Win32Thread;

// Heap allocated storage for thread start data passed to Thread::Create.
struct ThreadStartData {
  ref_ptr<Win32Thread> thread;
  std::function<void()> start_routine_fn;
  Thread::ThreadStartRoutine start_routine = nullptr;
  void* start_param = nullptr;
};

class Win32Thread : public Win32WaitHandle<Thread> {
 public:
  explicit Win32Thread(HANDLE handle, std::string name = "");

  ~Win32Thread() override;

  // Performs one-time thread init before running the thread start routine.
  // This is called on the thread itself.
  void OnEnter() override;

  // Performs one-time thread teardown after returning from the thread start
  // routine.
  // This is called on the thread itself after the thread start routine has
  // returned. Try not to do too much here, as the exact state of the thread
  // (especially with respect to other TLS values) is loosely defined.
  void OnExit() override;

  uintptr_t thread_id() override;
  bool is_current() const override;
  PriorityClass priority_class() const override;
  void set_priority_class(PriorityClass priority_class) override;
  uint64_t affinity_mask() const override;
  void set_affinity_mask(uint64_t affinity_mask) override;
  void Resume() override;

 private:
  friend class Thread;

  // Creates a new thread and passes it the given start data.
  static ref_ptr<Thread> CreateThread(
      const Thread::CreateParams& create_params,
      std::unique_ptr<ThreadStartData> start_data);
  // Runs the thread entry point specified by the Thread::Create call.
  static DWORD WINAPI ThreadStartRoutine(LPVOID param);

  static WaitAnyResult WaitMultiple(ArrayView<ref_ptr<WaitHandle>> wait_handles,
                                    std::chrono::milliseconds timeout,
                                    bool require_all);

  // There's no easy way to get this so we cache it. It's not thread safe but
  // the affinity mask should really only be specified on startup once.
  uint64_t affinity_mask_ = 0;

  // Whether the thread is currently suspended. All threads start suspended and
  // must be resumed.
  std::atomic<bool> suspended_{true};
};

// Ensures we have a TLS slot for the current thread.
// Safe to call multiple times.
void InitializeCurrentThreadStorage() {
  static std::once_flag current_thread_fls_index_flag;
  std::call_once(current_thread_fls_index_flag, []() {
    current_thread_fls_index_ = ::FlsAlloc([](void* data) {
      auto thread = reinterpret_cast<Win32Thread*>(data);
      if (thread) {
        // Tear down thread.
        thread->OnExit();

        // Release the reference to the thread owned by TLS.
        // This may delete the thread.
        thread->ReleaseReference();
      }
    });
  });
}

// Sets the name of the current thread as seen in the debugger, if one is
// attached. Unfortunately if a debugger is not attached at the time this is
// called it will remain unnamed.
void SetDebugThreadName(const char* name) {
  if (!::IsDebuggerPresent()) {
    return;
  }

#pragma pack(push, 8)
  // http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
  struct THREADNAME_INFO {
    DWORD dwType;      // Must be 0x1000.
    LPCSTR szName;     // Pointer to name (in user addr space).
    DWORD dwThreadID;  // Thread ID (-1=caller thread).
    DWORD dwFlags;     // Reserved for future use, must be zero.
  };
#pragma pack(pop)

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = -1;  // Current thread.
  info.dwFlags = 0;
  __try {
    ::RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR),
                     reinterpret_cast<ULONG_PTR*>(&info));
  } __except (EXCEPTION_EXECUTE_HANDLER) {  // NOLINT
  }
}

}  // namespace

int Process::logical_processor_count() {
  return std::thread::hardware_concurrency();
}

void Process::EnableHighResolutionTiming() {
  // This will change the timer resolution for the entire system.
  // https://msdn.microsoft.com/en-us/library/dd757624(v=vs.85).aspx
  MMRESULT result = ::timeBeginPeriod(kHighResolutionTimingPeriodMillis);
  if (result != TIMERR_NOERROR) {
    LOG(WARNING) << "Unable to enable high-resolution timing mode, "
                 << "timers will be wildly inaccurate!";
  }
}

void Process::DisableHighResolutionTiming() {
  // Accepts the same value we pass to timeBeginPeriod.
  ::timeEndPeriod(kHighResolutionTimingPeriodMillis);
}

ref_ptr<Thread> Thread::Create(const CreateParams& create_params,
                               std::function<void()> start_routine) {
  auto start_data = make_unique<ThreadStartData>();
  start_data->start_routine_fn = std::move(start_routine);
  return Win32Thread::CreateThread(create_params, std::move(start_data));
}

ref_ptr<Thread> Thread::Create(const CreateParams& create_params,
                               ThreadStartRoutine start_routine,
                               void* start_param) {
  auto start_data = make_unique<ThreadStartData>();
  start_data->start_routine = start_routine;
  start_data->start_param = start_param;
  return Win32Thread::CreateThread(create_params, std::move(start_data));
}

// Creates a new thread and passes it the given start data.
ref_ptr<Thread> Win32Thread::CreateThread(
    const CreateParams& create_params,
    std::unique_ptr<ThreadStartData> start_data) {
  // Create the thread now.
  // Note that we always create the thread suspended so we have time to
  // initialize the thread object.
  // If we didn't do this it's possible the OS could schedule the thread
  // immediately inside of CreateThread and we wouldn't be able to prepare it.
  HANDLE handle =
      ::CreateThread(nullptr, create_params.stack_size, ThreadStartRoutine,
                     start_data.get(), CREATE_SUSPENDED, nullptr);
  DCHECK_NE(handle, INVALID_HANDLE_VALUE);
  if (handle == INVALID_HANDLE_VALUE) {
    LOG(FATAL) << "Unable to create thread: " << ::GetLastError();
    return nullptr;
  }

  // Create our Thread and stash the reference in the start data.
  // When the thread spins up it will set the reference in its TLS.
  auto thread = make_ref<Win32Thread>(handle, create_params.name);
  start_data->thread = thread;

  // Set initial values.
  thread->set_priority_class(create_params.priority_class);
  if (create_params.affinity_mask) {
    thread->set_affinity_mask(create_params.affinity_mask);
  }

  // Release the start data, as it's now owned by the thread.
  start_data.release();

  // If we are not creating the thread suspended we can resume it now. We may
  // context switch into it immediately.
  if (!create_params.create_suspended) {
    thread->Resume();
  }

  return thread;
}

DWORD WINAPI Win32Thread::ThreadStartRoutine(LPVOID param) {
  std::unique_ptr<ThreadStartData> start_data(
      reinterpret_cast<ThreadStartData*>(param));
  DCHECK(start_data);

  // Retain the thread object on the stack here for the duration of the thread.
  auto self_thread = std::move(start_data->thread);

  // Prep the thread.
  self_thread->OnEnter();

  if (start_data->start_routine) {
    // Pull off the start routine and deallocate the start data.
    auto start_routine = start_data->start_routine;
    void* start_param = start_data->start_param;
    start_data.reset();

    // Run the thread start routine.
    start_routine(start_param);
  } else {
    // Pull off the start routine and deallocate the start data.
    auto start_routine = std::move(start_data->start_routine_fn);
    start_data.reset();

    // Run the thread start routine.
    start_routine();
  }

  // TLS teardown will call back the FlsAlloc function and issue OnExit.
  return 0;
}

ref_ptr<Thread> Thread::current_thread() {
  // Ensure TLS is setup.
  InitializeCurrentThreadStorage();

  // We implicitly create Threads to wrap existing Win32 threads when we are
  // first called on them.
  auto current_thread =
      reinterpret_cast<Thread*>(::FlsGetValue(current_thread_fls_index_));
  if (current_thread) {
    // We've already created a thread handle.
    return ref_ptr<Thread>(current_thread);
  }

  // Create a new thread handle.
  HANDLE handle = INVALID_HANDLE_VALUE;
  ::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(),
                    ::GetCurrentProcess(), &handle, 0, FALSE,
                    DUPLICATE_SAME_ACCESS);
  DCHECK_NE(handle, INVALID_HANDLE_VALUE);
  auto thread = make_ref<Win32Thread>(handle);

  // Perform thread init (such as storing the TLS reference).
  thread->OnEnter();

  return thread;
}

const std::string& Thread::name() {
  auto thread = Thread::current_thread();
  DCHECK(thread) << "No current Thread";
  return thread->name_;
}

void Thread::set_name(const std::string& name) {
  auto thread = Thread::current_thread();
  DCHECK(thread) << "No current Thread";
  thread->name_ = name;

  // TODO(benvanik): WTF.

  // Set the thread name shown in the debugger.
  SetDebugThreadName(name.c_str());
}

uintptr_t Thread::AllocateLocalStorageSlot(void (*release_callback)(void*)) {
  return static_cast<uintptr_t>(::FlsAlloc(release_callback));
}

void Thread::DeallocateLocalStorageSlot(uintptr_t slot_id) {
  // NOTE: destructors will be called!
  ::FlsFree(static_cast<DWORD>(slot_id));
}

void* Thread::GetLocalStorageSlotValue(uintptr_t slot_id) {
  return ::FlsGetValue(static_cast<DWORD>(slot_id));
}

void Thread::SetLocalStorageSlotValue(uintptr_t slot_id, void* value) {
  ::FlsSetValue(static_cast<DWORD>(slot_id), value);
}

void Thread::TryYield() { std::this_thread::yield(); }

void Thread::Sleep(std::chrono::microseconds duration) {
  if (duration.count() < 1000) {
    // Windows doesn't do well with very short Sleeps - trying to sleep for
    // 1 microsecond may take hundreds. We try to do what the caller expects
    // knowing that SwitchToThread does something on Windows by spin-waiting.
    Stopwatch stopwatch;
    do {
      TryYield();
    } while (stopwatch.elapsed_micros() < duration);
  } else {
    // Note: the Windows Sleep function is pretty bad.
    // See section 2.2 for more information:
    // http://www.windowstimestamp.com/description#C_2
    // This sleep could be anywhere between 0ms and 16ms off of the requested
    // amount (yes, Sleep(1) could be 16ms!), unless EnableHighResolutionTiming
    // has been called. When in high-resolution mode the sleep is much more
    // likely to be millisecond accurate (but still not guaranteed).
    ::Sleep(static_cast<DWORD>(duration.count() / 1000));
  }
}

Thread::WaitResult Thread::Wait(ref_ptr<WaitHandle> wait_handle,
                                std::chrono::milliseconds timeout) {
  DWORD result = ::WaitForSingleObjectEx(
      reinterpret_cast<HANDLE>(wait_handle->native_handle()),
      static_cast<DWORD>(timeout.count()), FALSE);
  switch (result) {
    case WAIT_OBJECT_0:
      return WaitResult::kSuccess;
    case WAIT_TIMEOUT:
      return WaitResult::kTimeout;
    case WAIT_ABANDONED:
    case WAIT_IO_COMPLETION:
      // NOTE: we don't support APC, and we shouldn't get abandoned handles.
      return WaitResult::kError;
    default:
    case WAIT_FAILED:
      return WaitResult::kError;
  }
}

Thread::WaitResult Thread::SignalAndWait(ref_ptr<WaitHandle> signal_handle,
                                         ref_ptr<WaitHandle> wait_handle,
                                         std::chrono::milliseconds timeout) {
  DWORD result = ::SignalObjectAndWait(
      reinterpret_cast<HANDLE>(signal_handle->native_handle()),
      reinterpret_cast<HANDLE>(wait_handle->native_handle()),
      static_cast<DWORD>(timeout.count()), FALSE);
  switch (result) {
    case WAIT_OBJECT_0:
      return WaitResult::kSuccess;
    case WAIT_TIMEOUT:
      return WaitResult::kTimeout;
    default:
    case WAIT_FAILED:
    case WAIT_ABANDONED:
    case WAIT_IO_COMPLETION:
      // NOTE: we don't support APC, and we shouldn't get abandoned handles.
      return WaitResult::kError;
  }
}

Thread::WaitAnyResult Thread::WaitAny(
    ArrayView<ref_ptr<WaitHandle>> wait_handles,
    std::chrono::milliseconds timeout) {
  return Win32Thread::WaitMultiple(wait_handles, timeout, false);
}

Thread::WaitResult Thread::WaitAll(ArrayView<ref_ptr<WaitHandle>> wait_handles,
                                   std::chrono::milliseconds timeout) {
  return Win32Thread::WaitMultiple(wait_handles, timeout, true).wait_result;
}

Thread::WaitAnyResult Win32Thread::WaitMultiple(
    ArrayView<ref_ptr<WaitHandle>> wait_handles,
    std::chrono::milliseconds timeout, bool require_all) {
  // NOTE: the wait handle count is limited so we can stack allocate.
  DCHECK_LE(wait_handles.size(), 64);
  HANDLE* handles =
      reinterpret_cast<HANDLE*>(_malloca(sizeof(HANDLE) * wait_handles.size()));
  for (size_t i = 0; i < wait_handles.size(); ++i) {
    handles[i] = reinterpret_cast<HANDLE>(wait_handles[i]->native_handle());
  }
  DWORD result = ::WaitForMultipleObjectsEx(
      static_cast<DWORD>(wait_handles.size()), handles,
      require_all ? TRUE : FALSE, static_cast<DWORD>(timeout.count()), FALSE);
  _freea(handles);
  if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + wait_handles.size()) {
    return {WaitResult::kSuccess, result - WAIT_OBJECT_0};
  } else if (result >= WAIT_ABANDONED_0 &&
             result < WAIT_ABANDONED_0 + wait_handles.size()) {
    // NOTE: we shouldn't get abandoned handles.
    return {WaitResult::kError, result - WAIT_ABANDONED_0};
  }
  switch (result) {
    case WAIT_TIMEOUT:
      return {WaitResult::kTimeout, 0};
    default:
    case WAIT_IO_COMPLETION:
    case WAIT_FAILED:
      // NOTE: we don't support APC.
      return {WaitResult::kError, 0};
  }
}

Win32Thread::Win32Thread(HANDLE handle, std::string name)
    : Win32WaitHandle(handle) {
  // Set a default thread name, if needed.
  name_ = std::move(name);
  if (name_.empty()) {
    // TODO(benvanik): better naming.
    name_ = std::string("Thread-") + std::to_string(thread_id());
  }

  // This must be called once on startup as the process affinity mask must be
  // initialized as the OS performs (thread mask & process mask) when setting
  // thread affinities.
  static std::once_flag initialize_affinity_mask_flag;
  static uint64_t initial_affinity_mask = 0;
  std::call_once(initialize_affinity_mask_flag, []() {
    HANDLE process_handle = ::GetCurrentProcess();
    DWORD_PTR process_affinity_mask = 0;
    DWORD_PTR system_affinity_mask = 0;
    ::GetProcessAffinityMask(process_handle, &process_affinity_mask,
                             &system_affinity_mask);
    ::SetProcessAffinityMask(process_handle, system_affinity_mask);
    initial_affinity_mask = process_affinity_mask;
  });

  // The thread starts with the same mask as the process.
  affinity_mask_ = initial_affinity_mask;
}

Win32Thread::~Win32Thread() {
  // WARNING: this may be called from any thread. Use OnExit to perform
  //          teardown on the thread during thread exit.
}

void Win32Thread::OnEnter() {
  // Ensure we have TLS setup.
  InitializeCurrentThreadStorage();

  // Stash a reference to the thread in TLS.
  // TLS owns a reference and it is cleaned up on thread exit.
  AddReference();
  ::FlsSetValue(current_thread_fls_index_, this);

  // Set initial name.
  Thread::set_name(name_);

  // Call base Thread enter handler.
  // We need to do this before we signal that startup has completed.
  Thread::OnEnter();
}

void Win32Thread::OnExit() {
  // Call base Thread exit handler.
  Thread::OnExit();
}

uintptr_t Win32Thread::thread_id() {
  return static_cast<uintptr_t>(::GetThreadId(handle_));
}

bool Win32Thread::is_current() const {
  return ::GetThreadId(handle_) == ::GetCurrentThreadId();
}

Thread::PriorityClass Win32Thread::priority_class() const {
  switch (::GetThreadPriority(handle_)) {
    case THREAD_PRIORITY_IDLE:
    case THREAD_PRIORITY_LOWEST:
      return PriorityClass::kLowest;
    case THREAD_PRIORITY_BELOW_NORMAL:
      return PriorityClass::kLow;
    default:
    case THREAD_PRIORITY_NORMAL:
      return PriorityClass::kNormal;
    case THREAD_PRIORITY_ABOVE_NORMAL:
      return PriorityClass::kHigh;
    case THREAD_PRIORITY_HIGHEST:
    case THREAD_PRIORITY_TIME_CRITICAL:
      return PriorityClass::kHighest;
  }
}

void Win32Thread::set_priority_class(PriorityClass priority_class) {
  DWORD priority = THREAD_PRIORITY_NORMAL;
  switch (priority_class) {
    case PriorityClass::kLowest:
      priority = THREAD_PRIORITY_LOWEST;
      break;
    case PriorityClass::kLow:
      priority = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case PriorityClass::kNormal:
      priority = THREAD_PRIORITY_NORMAL;
      break;
    case PriorityClass::kHigh:
      priority = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case PriorityClass::kHighest:
      priority = THREAD_PRIORITY_HIGHEST;
      break;
  }
  ::SetThreadPriority(handle_, priority);
}

uint64_t Win32Thread::affinity_mask() const { return affinity_mask_; }

void Win32Thread::set_affinity_mask(uint64_t affinity_mask) {
  affinity_mask_ = affinity_mask;
  ::SetThreadAffinityMask(handle_, affinity_mask_);
}

void Win32Thread::Resume() {
  if (suspended_.exchange(false) == true) {
    DWORD result = ::ResumeThread(handle_);
    if (result == UINT_MAX) {
      LOG(ERROR) << "Failed to resume thread";
    }
  }
}

}  // namespace xrtl
