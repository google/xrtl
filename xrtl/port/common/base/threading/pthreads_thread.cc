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

#include <pthread.h>
#include <sys/sysctl.h>
#include <time.h>
#include <unistd.h>

#include <mutex>

#include "xrtl/tools/target_platform/target_platform.h"

#if defined(XRTL_PLATFORM_APPLE)
#include <mach/mach.h>
#include <mach/thread_act.h>
#elif defined(XRTL_PLATFORM_LINUX)
#include <sys/resource.h>  // setpriority
#include <sys/syscall.h>   // gettid
#endif                     // XRTL_PLATFORM_APPLE

#include "xrtl/base/logging.h"
#include "xrtl/base/stopwatch.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/port/common/base/threading/pthreads_wait_handle.h"

namespace xrtl {

namespace {

// TLS slot that holds the current Thread* value.
// The slot owns a reference so that the Thread stays alive so long as the value
// is set. When the thread exits the reference is cleared up automatically.
pthread_key_t current_thread_key_ = 0;

class PthreadsThread;

// Heap allocated storage for thread start data passed to Thread::Create.
struct ThreadStartData {
  ref_ptr<PthreadsThread> thread;
  std::function<void()> start_routine_fn;
  Thread::ThreadStartRoutine start_routine = nullptr;
  void* start_param = nullptr;
};

class PthreadsThread : public PthreadsWaitHandle<Thread> {
 public:
  explicit PthreadsThread(pthread_t handle, std::string name = "");

  ~PthreadsThread() override;

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
      const CreateParams& create_params,
      std::unique_ptr<ThreadStartData> start_data);
  // Runs the thread entry point specified by the Thread::Create call.
  static void* ThreadStartRoutine(void* param);

  static WaitAnyResult WaitMultiple(ArrayView<ref_ptr<WaitHandle>> wait_handles,
                                    std::chrono::milliseconds timeout,
                                    bool require_all);

  bool CheckCondition() const override { return zombie_; }

  void SetWaitSuccessful() override {
    // This gets called with the wait lock held, meaning that no one else can
    // wait on us and there's at least one reference still alive.
    // When a wait succeeds on a thread it means it's ended. The perfect time
    // to join!
    // Note that calling pthread_join multiple times is undefined, so we guard
    // that with a flag.
    if (!has_joined_.exchange(true)) {
      pthread_join(handle_, nullptr);
    }
  }

  // pthreads thread handle.
  pthread_t handle_ = 0;
  // System thread ID (tid).
  uintptr_t thread_id_ = -1;

  // Current thread priority.
  std::atomic<PriorityClass> priority_class_{PriorityClass::kNormal};

  // Set true when the thread exits. This is used by the wait handle for joining
  // with the thread.
  std::atomic<bool> zombie_{false};
  // Set true when pthread_join has been called.
  std::atomic<bool> has_joined_{false};

  // An event signaled by the thread when it has completed OnEnter.
  ref_ptr<Event> startup_fence_;
  // An event signaled when the thread has been resumed.
  // This will keep the thread in its start function waiting until the condition
  // is set.
  ref_ptr<Event> suspend_fence_;
};

// Ensures we have a TLS slot for the current thread.
// Safe to call multiple times.
void InitializeCurrentThreadStorage() {
  static std::once_flag current_thread_key_flag;
  std::call_once(current_thread_key_flag, []() {
    pthread_key_create(&current_thread_key_, [](void* data) {
      auto thread = reinterpret_cast<PthreadsThread*>(data);
      if (thread) {
        // Tear down thread and possibly delete this;.
        thread->OnExit();
      }
    });
  });
}

// Converts a relative timeout (like 100ms) to an absolute time.
// This ensures that wait loops are always timed out at the originally intended
// time regardless of how many times they wake.
timespec ConvertRelativeTimeoutToAbsolute(std::chrono::microseconds duration) {
  auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nsecs =
      std::chrono::duration_cast<std::chrono::nanoseconds>(duration - secs);
  timespec ts = {0};
  clock_gettime(CLOCK_REALTIME, &ts);
  secs += std::chrono::seconds(ts.tv_sec);
  nsecs += std::chrono::nanoseconds(ts.tv_nsec);
  while (nsecs.count() >= 1000000000) {
    nsecs -= std::chrono::nanoseconds(1000000000);
    ++secs;
  }
  ts.tv_sec = secs.count();
  ts.tv_nsec = nsecs.count();
  return ts;
}

}  // namespace

int Process::logical_processor_count() {
#if defined(XRTL_PLATFORM_APPLE)
  int value = 1;
  size_t value_length = sizeof(value);
  if (sysctlbyname("hw.logicalcpu", &value, &value_length, nullptr, 0) != 0) {
    if (sysctlbyname("hw.ncpu", &value, &value_length, nullptr, 0) != 0) {
      return 1;
    }
  }
  return value;
#elif defined(XRTL_PLATFORM_EMSCRIPTEN)
  return 1;
#else
#if defined(_SC_NPROCESSORS_ONLN)
  return sysconf(_SC_NPROCESSORS_ONLN);
#else
  return sysconf(_SC_NPROC_ONLN);
#endif  // _SC_NPROCESSORS_ONLN
#endif  // XRTL_PLATFORM_APPLE
}

void Process::EnableHighResolutionTiming() {
  // No-op, AFAIK.
}

void Process::DisableHighResolutionTiming() {
  // No-op, AFAIK.
}

ref_ptr<Thread> Thread::Create(const CreateParams& create_params,
                               std::function<void()> start_routine) {
  auto start_data = make_unique<ThreadStartData>();
  start_data->start_routine_fn = std::move(start_routine);
  return PthreadsThread::CreateThread(create_params, std::move(start_data));
}

ref_ptr<Thread> Thread::Create(const CreateParams& create_params,
                               ThreadStartRoutine start_routine,
                               void* start_param) {
  auto start_data = make_unique<ThreadStartData>();
  start_data->start_routine = start_routine;
  start_data->start_param = start_param;
  return PthreadsThread::CreateThread(create_params, std::move(start_data));
}

ref_ptr<Thread> PthreadsThread::CreateThread(
    const CreateParams& create_params,
    std::unique_ptr<ThreadStartData> start_data) {
  // Create our Thread and stash the reference in the start data.
  // When the thread spins up it will set the reference in its TLS and populate
  // handle_ with a real handle. For now, we just pass an invalid (0) handle.
  pthread_t invalid_handle;
  std::memset(&invalid_handle, 0, sizeof(invalid_handle));
  auto thread = make_ref<PthreadsThread>(invalid_handle, create_params.name);
  start_data->thread = thread;

  // Create the thread now.
  // Note that we always create the thread suspended so we have time to
  // initialize the thread object.
  // If we didn't do this it's possible the OS could schedule the thread
  // immediately inside of pthread_create and we wouldn't be able to prepare it.
  pthread_attr_t thread_attr = {};
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
  if (create_params.stack_size) {
    pthread_attr_setstacksize(&thread_attr, create_params.stack_size);
  }
#if defined(XRTL_PLATFORM_APPLE)
  // Always create threads suspended.
  int rc = pthread_create_suspended_np(&thread->handle_, &thread_attr,
                                       &ThreadStartRoutine, start_data.get());
  if (rc == 0) {
    thread->thread_id_ = pthread_mach_thread_np(thread->handle_);
  }
#else
  // No support for actual create-suspended, so the fence is all we got.
  // This just means we'll get one additional spurious wake of the new thread
  // on startup, which isn't optimal but still safe due to our fence.
  int rc = pthread_create(&thread->handle_, &thread_attr, &ThreadStartRoutine,
                          start_data.get());
  if (rc == 0) {
    // Wait for the thread to start up and get its handle initialized.
    Thread::Wait(thread->startup_fence_);
  }
#endif  // XRTL_PLATFORM_APPLE
  pthread_attr_destroy(&thread_attr);

  DCHECK_EQ(rc, 0);
  if (rc != 0) {
    LOG(FATAL) << "Unable to create thread: " << rc;
    return nullptr;
  }

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

void* PthreadsThread::ThreadStartRoutine(void* param) {
  std::unique_ptr<ThreadStartData> start_data(
      reinterpret_cast<ThreadStartData*>(param));
  DCHECK(start_data);

  // Retain the thread object on the stack here for the duration of the thread.
  auto self_thread = std::move(start_data->thread);

  // Prep the thread.
  self_thread->OnEnter();

  if (!self_thread->zombie_) {
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
  }

  // TLS teardown will call back the FlsAlloc function and issue OnExit.
  return nullptr;
}

ref_ptr<Thread> Thread::current_thread() {
  // Ensure TLS is setup.
  InitializeCurrentThreadStorage();

  // We implicitly create Threads to wrap existing threads when we are first
  // called on them.
  auto current_thread =
      reinterpret_cast<Thread*>(pthread_getspecific(current_thread_key_));
  if (current_thread) {
    // We've already created a thread handle.
    return ref_ptr<Thread>(current_thread);
  }

  // Create a new thread handle for this thread.
  // Implicitly created threads don't start suspended.
  auto thread = make_ref<PthreadsThread>(pthread_self());
  thread->Resume();

  // Perform thread init (such as storing the TLS reference).
  thread->OnEnter();

  // TODO(benvanik): atexit handler that runs through a list of implicit threads
  //                 and releases them? Or, we could mark this implicit thread
  //                 as unchecked in heap-checker.

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
#if defined(XRTL_PLATFORM_APPLE)
  pthread_setname_np(name.c_str());
#else
  pthread_setname_np(pthread_self(), name.c_str());
#endif  // XRTL_PLATFORM_APPLE
}

uintptr_t Thread::AllocateLocalStorageSlot(void (*release_callback)(void*)) {
  pthread_key_t key = 0;
  pthread_key_create(&key, release_callback);
  return static_cast<uintptr_t>(key);
}

void Thread::DeallocateLocalStorageSlot(uintptr_t slot_id) {
  // NOTE: no destructors will be called!
  pthread_key_delete(static_cast<pthread_key_t>(slot_id));
}

void* Thread::GetLocalStorageSlotValue(uintptr_t slot_id) {
  return pthread_getspecific(static_cast<pthread_key_t>(slot_id));
}

void Thread::SetLocalStorageSlotValue(uintptr_t slot_id, void* value) {
  pthread_setspecific(static_cast<pthread_key_t>(slot_id), value);
}

void Thread::TryYield() {
#if defined(XRTL_PLATFORM_APPLE)
  pthread_yield_np();
#else
  pthread_yield();
#endif  // XRTL_PLATFORM_APPLE
}

void Thread::Sleep(std::chrono::microseconds duration) {
  timespec rqtp = {
      std::chrono::duration_cast<std::chrono::seconds>(duration).count(),
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::microseconds(duration.count() % 1000000))
          .count()};
  // TODO(benvanik): if rc == -1 we should probably loop. Would need to adjust.
  nanosleep(&rqtp, nullptr);
}

Thread::WaitResult Thread::Wait(ref_ptr<WaitHandle> wait_handle,
                                std::chrono::milliseconds timeout) {
  wait_handle = wait_handle;  // Keep analysis happy.

  // We only support pthreads wait handles.
  auto pthreads_wait_handle =
      reinterpret_cast<PthreadsWaitHandleImpl*>(wait_handle->native_handle());
  pthread_mutex_t* wait_mutex = pthreads_wait_handle->wait_mutex();
  pthread_cond_t* wait_cond = pthreads_wait_handle->wait_cond();

  WaitResult result = WaitResult::kSuccess;
  if (timeout == kImmediateTimeout) {
    // Wait never.
    pthread_mutex_lock(wait_mutex);
    if (pthreads_wait_handle->CheckCondition()) {
      result = WaitResult::kSuccess;
      pthreads_wait_handle->SetWaitSuccessful();
    } else {
      result = WaitResult::kTimeout;
    }
    pthread_mutex_unlock(wait_mutex);
  } else if (timeout == kInfiniteTimeout) {
    // Wait forever.
    pthread_mutex_lock(wait_mutex);
    while (!pthreads_wait_handle->CheckCondition() &&
           result == WaitResult::kSuccess) {
      int rc = pthread_cond_wait(wait_cond, wait_mutex);
      if (rc == 0) {
        result = WaitResult::kSuccess;
      } else {
        result = WaitResult::kError;
      }
    }
    if (result == WaitResult::kSuccess) {
      pthreads_wait_handle->SetWaitSuccessful();
    }
    pthread_mutex_unlock(wait_mutex);
  } else {
    // Wait with timeout support.
    timespec ts = ConvertRelativeTimeoutToAbsolute(timeout);
    pthread_mutex_lock(wait_mutex);
    while (!pthreads_wait_handle->CheckCondition() &&
           result == WaitResult::kSuccess) {
      int rc = pthread_cond_timedwait(wait_cond, wait_mutex, &ts);
      if (rc == 0) {
        result = WaitResult::kSuccess;
      } else if (rc == ETIMEDOUT) {
        result = WaitResult::kTimeout;
      } else {
        result = WaitResult::kError;
      }
    }
    if (result == WaitResult::kSuccess) {
      pthreads_wait_handle->SetWaitSuccessful();
    }
    pthread_mutex_unlock(wait_mutex);
  }
  return result;
}

Thread::WaitResult Thread::SignalAndWait(ref_ptr<WaitHandle> signal_handle,
                                         ref_ptr<WaitHandle> wait_handle,
                                         std::chrono::milliseconds timeout) {
  signal_handle = signal_handle;  // Keep analysis happy.

  // Nothing fancy, just signal + wait.
  // We only support pthreads wait handles.
  auto pthreads_signal_handle =
      reinterpret_cast<PthreadsWaitHandleImpl*>(signal_handle->native_handle());
  if (!pthreads_signal_handle->Signal()) {
    return WaitResult::kError;
  }
  return Wait(std::move(wait_handle), timeout);
}

Thread::WaitAnyResult Thread::WaitAny(
    ArrayView<ref_ptr<WaitHandle>> wait_handles,
    std::chrono::milliseconds timeout) {
  return PthreadsThread::WaitMultiple(wait_handles, timeout, false);
}

Thread::WaitResult Thread::WaitAll(ArrayView<ref_ptr<WaitHandle>> wait_handles,
                                   std::chrono::milliseconds timeout) {
  return PthreadsThread::WaitMultiple(wait_handles, timeout, true).wait_result;
}

Thread::WaitAnyResult PthreadsThread::WaitMultiple(
    ArrayView<ref_ptr<WaitHandle>> wait_handles,
    std::chrono::milliseconds timeout, bool require_all) {
  // pthreads has no way of doing multi-waits, so our performance won't be as
  // good as on systems that do support it. That's generally ok, as multi-waits
  // are rare.
  // The multi-wait is implemented by waiting on a shared condvar that is pulsed
  // every time a wait completes. This means that all threads performing
  // multi-waits will wake and loop on any signaling of any handle regardless of
  // whether it's waiting on it. Eh. Use epoll/kqueue implementations for perf.

  WaitAnyResult result = {WaitResult::kSuccess, 0};

  // Calculate timeout, if any.
  timespec ts;
  if (timeout != kImmediateTimeout && timeout != kInfiniteTimeout) {
    ts = ConvertRelativeTimeoutToAbsolute(timeout);
  }

  // Copy wait handles locally. We'll use this as a check and null out entries
  // that have passed and have been signaled.
  // NOTE: the wait handle count is limited so we can stack allocate.
  DCHECK_LE(wait_handles.size(), 64);
  PthreadsWaitHandleImpl** handles = reinterpret_cast<PthreadsWaitHandleImpl**>(
      alloca(sizeof(PthreadsWaitHandleImpl*) * wait_handles.size()));
  for (size_t i = 0; i < wait_handles.size(); ++i) {
    handles[i] = reinterpret_cast<PthreadsWaitHandleImpl*>(
        wait_handles[i]->native_handle());
  }

  // Master loop.
  pthread_mutex_lock(shared_multi_mutex());
  while (result.wait_result == WaitResult::kSuccess) {
    // Check all handles for completion.
    int signal_index = -1;
    bool any_signaled = false;
    bool any_unsignaled = false;
    for (size_t i = 0; i < wait_handles.size(); ++i) {
      PthreadsWaitHandleImpl* handle = handles[i];
      if (!handle) {
        signal_index = i;
        any_signaled = true;
        continue;
      }
      pthread_mutex_lock(handle->wait_mutex());
      if (handle->CheckCondition()) {
        signal_index = i;
        any_signaled = true;
        handle->SetWaitSuccessful();
        handles[i] = nullptr;
      } else {
        any_unsignaled = true;
      }
      pthread_mutex_unlock(handle->wait_mutex());
      if (any_signaled && !require_all) {
        break;
      }
    }
    if (require_all && !any_unsignaled) {
      // Waiting for all and all are signaled.
      result = {WaitResult::kSuccess, 0};
      break;
    } else if (!require_all && any_signaled) {
      // Waiting for only one to complete; are done!
      result = {WaitResult::kSuccess, static_cast<size_t>(signal_index)};
      break;
    }

    // We aren't satisfied yet so let's wait until something signals.
    if (timeout == kImmediateTimeout) {
      // Actually, the user doesn't want to wait - so let's just bail.
      result = {WaitResult::kTimeout, 0};
    } else if (timeout == kInfiniteTimeout) {
      // Spin and wait for events to complete.
      int rc = pthread_cond_wait(shared_multi_cond(), shared_multi_mutex());
      if (rc != 0) {
        result = {WaitResult::kError, 0};
      }
    } else {
      // Spin with a timeout.
      int rc = pthread_cond_timedwait(shared_multi_cond(), shared_multi_mutex(),
                                      &ts);
      if (rc == ETIMEDOUT) {
        result = {WaitResult::kTimeout, 0};
      } else if (rc != 0) {
        result = {WaitResult::kError, 0};
      }
    }
  }
  pthread_mutex_unlock(shared_multi_mutex());

  return result;
}

PthreadsThread::PthreadsThread(pthread_t handle, std::string name)
    : PthreadsWaitHandle(), handle_(handle) {
  // Set a default thread name, if needed.
  name_ = std::move(name);
  if (name_.empty()) {
    // TODO(benvanik): better naming.
    name_ =
        std::string("Thread-") + std::to_string(PthreadsThread::thread_id());
  }

  // The thread create function will wait until we set this in OnEnter (on some
  // platforms).
  startup_fence_ = Event::CreateManualResetEvent(false);
  // The thread start routine will pause until we set this in Resume.
  suspend_fence_ = Event::CreateManualResetEvent(false);
}

PthreadsThread::~PthreadsThread() {
  // WARNING: this may be called from any thread. Use OnExit to perform
  //          teardown on the thread during thread exit.

  // If we never joined we should detach now to ensure we don't leak the thread.
  if (!has_joined_) {
    pthread_detach(handle_);
  }
}

void PthreadsThread::OnEnter() {
  // There's a race between the creating thread and this for setting handle_,
  // but the value should be the same between the two so it's fine.
  handle_ = pthread_self();

#if defined(XRTL_PLATFORM_APPLE)
  // thread_id_ already set after creation.
  DCHECK_NE(thread_id_, 0);
#elif defined(XRTL_PLATFORM_LINUX)
  // Linux needs the thread ID (tid) for priority manipulation.
  thread_id_ = syscall(SYS_gettid);
#else
  thread_id_ = static_cast<uintptr_t>(handle_);
#endif  // XRTL_PLATFORM_LINUX

  // Ensure we have TLS setup.
  InitializeCurrentThreadStorage();

  // Stash a reference to the thread in TLS.
  // TLS owns a reference and it is cleaned up on thread exit.
  AddReference();
  pthread_setspecific(current_thread_key_, this);

  // Set initial name.
  Thread::set_name(name_);

  // Call base Thread enter handler.
  // We need to do this before we signal that startup has completed.
  Thread::OnEnter();

  // Thread is ready, signal creator and wait until we are resumed.
  Thread::SignalAndWait(startup_fence_, suspend_fence_);
}

void PthreadsThread::OnExit() {
  DCHECK_EQ(handle_, pthread_self());
  pthread_mutex_lock(wait_mutex());
  bool was_zombie = zombie_;
  pthread_mutex_unlock(wait_mutex());
  if (was_zombie) {
    return;
  }

  // Call base Thread exit handler.
  // We need to do this before we actually bring down the thread and notify
  // waiters.
  Thread::OnExit();

  // Signal thread exit. This will likely wake waiters.
  pthread_mutex_lock(wait_mutex());
  zombie_ = true;
  pthread_cond_broadcast(wait_cond());          // Wake all waiters.
  pthread_cond_broadcast(shared_multi_cond());  // Wake all multi-waiters.
  pthread_mutex_unlock(wait_mutex());

  // Drop the reference that TLS was keeping.
  ReleaseReference();
}

uintptr_t PthreadsThread::thread_id() {
  return static_cast<uintptr_t>(thread_id_);
}

bool PthreadsThread::is_current() const { return handle_ == pthread_self(); }

#if defined(XRTL_PLATFORM_APPLE)
namespace {

struct SchedulerPriorities {
  int lowest_priority = 0;
  int low_priority = 0;
  int normal_priority = 0;
  int high_priority = 0;
  int highest_priority = 0;
};

// Returns a structure containing our idea of OS scheduler priorities as they
// map to our PriorityClass.
SchedulerPriorities CalculateSchedulerPriorities(int policy) {
  int min_priority = sched_get_priority_min(policy);
  int max_priority = sched_get_priority_max(policy);
  SchedulerPriorities result;
  result.lowest_priority = min_priority;
  result.normal_priority = (max_priority - min_priority) / 2 + min_priority;
  result.highest_priority = max_priority;
  result.low_priority =
      (result.normal_priority - min_priority) / 2 + min_priority;
  result.high_priority =
      (max_priority - result.normal_priority) / 2 + result.normal_priority;
  return result;
}

}  // namespace
#endif  // XRTL_PLATFORM_APPLE

Thread::PriorityClass PthreadsThread::priority_class() const {
#if defined(XRTL_PLATFORM_APPLE)
  int policy = 0;
  sched_param param = {0};
  pthread_getschedparam(handle_, &policy, &param);
  auto priorities = CalculateSchedulerPriorities(policy);
  if (param.sched_priority <= priorities.lowest_priority) {
    return PriorityClass::kLowest;
  } else if (param.sched_priority <= priorities.low_priority) {
    return PriorityClass::kLow;
  } else if (param.sched_priority < priorities.high_priority) {
    return PriorityClass::kNormal;
  } else if (param.sched_priority < priorities.highest_priority) {
    return PriorityClass::kHigh;
  } else {
    return PriorityClass::kHighest;
  }
#elif defined(XRTL_PLATFORM_LINUX)
  return priority_class_;
#else
#error Platform not yet implemented
#endif  // XRTL_PLATFORM_APPLE
}

void PthreadsThread::set_priority_class(PriorityClass priority_class) {
  priority_class_ = priority_class;
#if defined(XRTL_PLATFORM_APPLE)
  int policy = 0;
  sched_param param = {0};
  pthread_getschedparam(handle_, &policy, &param);
  auto priorities = CalculateSchedulerPriorities(policy);
  switch (priority_class) {
    case PriorityClass::kLowest:
      param.sched_priority = priorities.lowest_priority;
      break;
    case PriorityClass::kLow:
      param.sched_priority = priorities.low_priority;
      break;
    case PriorityClass::kNormal:
      param.sched_priority = priorities.normal_priority;
      break;
    case PriorityClass::kHigh:
      param.sched_priority = priorities.high_priority;
      break;
    case PriorityClass::kHighest:
      param.sched_priority = priorities.highest_priority;
      break;
  }
  pthread_setschedparam(handle_, policy, &param);
#elif defined(XRTL_PLATFORM_LINUX)
  // I have no idea. getpriority/setpriority(gettid()) seem busted.
  if (priority_class != PriorityClass::kNormal) {
    LOG(WARNING) << "Ignoring thread priority change request to "
                 << static_cast<int>(priority_class);
  }
#else
#error Platform not yet implemented
#endif  // XRTL_PLATFORM_APPLE
}

uint64_t PthreadsThread::affinity_mask() const {
#if defined(XRTL_PLATFORM_APPLE)
  thread_affinity_policy_data_t policy_data;
  mach_msg_type_number_t policy_count = THREAD_AFFINITY_POLICY_COUNT;
  boolean_t is_default = false;
  thread_policy_get(pthread_mach_thread_np(handle_), THREAD_AFFINITY_POLICY,
                    reinterpret_cast<thread_policy_t>(&policy_data),
                    &policy_count, &is_default);
  return policy_data.affinity_tag;
#elif defined(XRTL_PLATFORM_LINUX)
  cpu_set_t cpu_set = {};
  CPU_ZERO(&cpu_set);
  pthread_getaffinity_np(handle_, sizeof(cpu_set), &cpu_set);
  uint64_t affinity_mask = 0;
  for (int cpu_index = 0; cpu_index < std::min(CPU_SETSIZE, 64); ++cpu_index) {
    if (CPU_ISSET(cpu_index, &cpu_set)) {
      affinity_mask |= (1ull << cpu_index);
    }
  }
  return affinity_mask;
#else
#error Platform not yet implemented
#endif  // XRTL_PLATFORM_APPLE
}

void PthreadsThread::set_affinity_mask(uint64_t affinity_mask) {
#if defined(XRTL_PLATFORM_APPLE)
  // NOTE: this sets a tag, not a CPU mask, so it'll behave differently than
  //       expected if there are many overlapping masks in use by the app.
  // https://developer.apple.com/library/content/releasenotes/Performance/RN-AffinityAPI/#//apple_ref/doc/uid/TP40006635-CH1-DontLinkElementID_2
  thread_affinity_policy_data_t policy_data = {
      static_cast<integer_t>(affinity_mask)};
  thread_policy_set(pthread_mach_thread_np(handle_), THREAD_AFFINITY_POLICY,
                    reinterpret_cast<thread_policy_t>(&policy_data),
                    THREAD_AFFINITY_POLICY_COUNT);
#elif defined(XRTL_PLATFORM_LINUX)
  cpu_set_t cpu_set = {};
  CPU_ZERO(&cpu_set);
  for (int cpu_index = 0; cpu_index < std::min(CPU_SETSIZE, 64); ++cpu_index) {
    if ((affinity_mask & (1ull << cpu_index)) != 0) {
      CPU_SET(cpu_index, &cpu_set);
    }
  }
  pthread_setaffinity_np(handle_, sizeof(cpu_set), &cpu_set);
#else
#error Platform not yet implemented
#endif  // XRTL_PLATFORM_APPLE
}

void PthreadsThread::Resume() {
  suspend_fence_->Set();

#if defined(XRTL_PLATFORM_APPLE)
  thread_resume(pthread_mach_thread_np(handle_));
#else
// No support for actual create-suspended, so the fence is all we got.
#endif  // XRTL_PLATFORM_APPLE
}

}  // namespace xrtl
