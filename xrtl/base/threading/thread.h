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

#ifndef XRTL_BASE_THREADING_THREAD_H_
#define XRTL_BASE_THREADING_THREAD_H_

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/macros.h"
#include "xrtl/base/threading/wait_handle.h"

namespace xrtl {

// When passed to a wait function the wait will immediately return instead of
// waiting. This can be used to query whether the wait would have occurred.
constexpr std::chrono::milliseconds kImmediateTimeout =
    std::chrono::milliseconds(0);
// When passed to a wait function the wait will never time out.
constexpr std::chrono::milliseconds kInfiniteTimeout =
    std::chrono::milliseconds::max();

// Utilities for the current process and other thread-shared stuff.
class Process {
 public:
  // Returns the total number of logical processors available.
  // This may include those enabled by hyperthreading. For example, a 1 package
  // 4 hardware core CPU with hyperthreading enabled would return as 8 logical
  // processor cores.
  // If you find this limiting add a physical_processor_count for the
  // non-hyperthreaded cores.
  static int logical_processor_count();

  // Requests the process enter a high-resolution timing mode.
  // This causes clocks and time-based functions (such as timeouts or sleeps) to
  // act at a higher resolution at the cost of more power burned.
  // When the application no longer needs high-resolution timing it should
  // return back to the default with DisableHighResolutionTiming.
  static void EnableHighResolutionTiming();
  static void DisableHighResolutionTiming();
};

// Represents a thread and provides utilities for threads.
// Many thread methods are only valid for the currently executing thread, such
// as set_name. Others are safe to call from any thread at any time.
//
// Threads are waitable objects and they are signaled when the thread has
// exited. In addition to calling Wait on the thread a Join helper exists that
// performs this for you.
class Thread : public WaitHandle {
 public:
  using ThreadStartRoutine = void (*)(void* data);

  // A thread priority class.
  // These translate roughly to the same thing across all platforms, though they
  // are just a hint and the schedulers on various platforms may behave very
  // differently. When in doubt prefer to write code that works at the extremes
  // of the classes.
  enum class PriorityClass : int32_t {
    // Lowest possible priority used for background threads that should never
    // block other threads.
    kLowest = -2,
    // Low priority, such as IO.
    kLow = -1,
    // Normal/default priority for the system.
    kNormal = 0,
    // High priority, such as audio processing.
    kHigh = 1,
    // Highest possible priority used for high resolution timers and signaling.
    kHighest = 2,
  };

  // Parameters for Thread::Create.
  struct CreateParams {
    // Name for the thread. May be changed later from within the thread with
    // set_name. If omitted a default name will be chosen.
    std::string name;

    // Stack size of the new thread, in bytes. If omitted a platform-defined
    // default stack size will be used.
    size_t stack_size = 0;

    // Whether to create the thread in a suspended state. The thread will be
    // initialized but not call the start_routine until it is resumed with
    // Resume.
    bool create_suspended = false;

    // Initial priority class.
    // This may be changed later via set_priority_class.
    PriorityClass priority_class = PriorityClass::kNormal;

    // Initial affinity mask or 0 to use the default.
    // This may be changed later via set_affinity_mask.
    uint64_t affinity_mask = 0;
  };

  // Creates a new thread with the given parameters and calls the start routine.
  static ref_ptr<Thread> Create(const CreateParams& create_params,
                                std::function<void()> start_routine);
  static ref_ptr<Thread> Create(const CreateParams& create_params,
                                ThreadStartRoutine start_routine,
                                void* start_param);

  // Returns a pointer to the current thread.
  static ref_ptr<Thread> current_thread();

  // Returns the name of the thread as assigned by set_name, or a default
  // value based on the thread_id.
  static const std::string& name();
  // Sets the name of the thread as it will appear in the debugger and logs.
  static void set_name(const std::string& name);

  // A thread-local storage slot.
  //
  // Usage:
  //  LocalStorageSlot<MyType> slot([](MyType* value) {
  //    delete value;
  //  });
  //  slot.set_value(new MyType());
  //  slot.value()->Foo();
  template <typename T>
  class LocalStorageSlot {
   public:
    using ReleaseCallback = void (*)(T*);

    LocalStorageSlot() {
      slot_id_ = Thread::AllocateLocalStorageSlot([](void* value) {});
    }

    // Allocates a local storage slot with a release callback.
    // The callback will be issued only when threads with values stored in the
    // slots are exited. It may not be called if the LocalStorageSlot is
    // deallocated, and will not be called when new values are set with
    // set_value.
    explicit LocalStorageSlot(ReleaseCallback release_callback) {
      slot_id_ = Thread::AllocateLocalStorageSlot(
          reinterpret_cast<void (*)(void*)>(release_callback));
    }

    ~LocalStorageSlot() { Thread::DeallocateLocalStorageSlot(slot_id_); }

    // Returns the value of the calling thread's local storage slot.
    T* value() const {
      return reinterpret_cast<T*>(Thread::GetLocalStorageSlotValue(slot_id_));
    }

    // Sets the value of the calling thread's local storage slot.
    // The release callback will not be made for existing values, if any were
    // set.
    void set_value(T* value) {
      Thread::SetLocalStorageSlotValue(slot_id_, value);
    }

   private:
    uintptr_t slot_id_ = 0;
  };

  // Yields execution back to the system thread scheduler.
  // This is just a hint and may have no effect. It should be used only when
  // very short sleeps are required (such as in a CAS loop), as otherwise it
  // will burn cycles.
  static void TryYield();

  // Sleeps the current thread for at least as long as the given duration.
  // Depending on the platform this may round up quite a bit.
  static void Sleep(std::chrono::microseconds duration);
  template <typename Rep, typename Period>
  static void Sleep(std::chrono::duration<Rep, Period> duration) {
    Sleep(std::chrono::duration_cast<std::chrono::microseconds>(duration));
  }

  // Describes the reason why a wait function returned.
  enum class WaitResult {
    // The object(s) being waited on were signaled.
    kSuccess = 0,
    // The timeout period elapsed without the object(s) being signaled.
    kTimeout,
    // The wait failed, usually due to invalid handles.
    kError,
  };

  // Waits until the given wait handle is in the signaled state or the timeout
  // interval elapses.
  //
  // If timeout is kImmediateTimeout the call will return immediately instead of
  // waiting if it would have waited. If timeout is kInfiniteTimeout the wait
  // will not time out.
  // Returns a result that indicates the reason the wait ended.
  static WaitResult Wait(ref_ptr<WaitHandle> wait_handle,
                         std::chrono::milliseconds timeout = kInfiniteTimeout);

  // Tries to wait on the given wait handle but immediately returns if the
  // thread would have blocked. This is equivalent to calling Wait with an
  // immediate timeout.
  // Returns true if the wait succeeded and false if the operation would have
  // blocked or failed.
  static bool TryWait(ref_ptr<WaitHandle> wait_handle) {
    return Wait(std::move(wait_handle), kImmediateTimeout) ==
           WaitResult::kSuccess;
  }

  // Signals one wait handle and waits on another as a single operation.
  // This acts as a Set when signal_handle is an Event and Release when it is
  // a Semaphore.
  // This can be more efficient than performing a Set/Release + Wait as the
  // platform scheduler can avoid yielding the thread immediately after Set.
  //
  // If timeout is kImmediateTimeout the call will return immediately instead of
  // waiting if it would have waited. If timeout is kInfiniteTimeout the wait
  // will not time out.
  // Returns a result that indicates the reason the wait ended.
  static WaitResult SignalAndWait(
      ref_ptr<WaitHandle> signal_handle, ref_ptr<WaitHandle> wait_handle,
      std::chrono::milliseconds timeout = kInfiniteTimeout);

  // Describes the result of a WaitAny function.
  struct WaitAnyResult {
    // The reason why the wait returned.
    WaitResult wait_result;
    // The index of the wait handle that caused the wait to be satisfied, if
    // wait_result is kSuccess.
    size_t wait_handle_index;
  };

  // Waits until any one of the specified wait handles are in the signaled
  // state or the timeout interval elapses.
  //
  // If timeout is kImmediateTimeout the call will return immediately instead of
  // waiting if it would have waited. If timeout is kInfiniteTimeout the wait
  // will not time out.
  // Returns a result containing the reason the wait ended and when successful
  // the index into wait_handles that was signaled.
  static WaitAnyResult WaitAny(
      ArrayView<ref_ptr<WaitHandle>> wait_handles,
      std::chrono::milliseconds timeout = kInfiniteTimeout);

  // Waits until all of the specified wait handles are in the signaled state or
  // the timeout interval elapses.
  //
  // If timeout is kImmediateTimeout the call will return immediately instead of
  // waiting if it would have waited. If timeout is kInfiniteTimeout the wait
  // will not time out.
  // Returns a result that indicates the reason the wait ended, with kSuccess
  // meaning that all objects were signaled.
  static WaitResult WaitAll(
      ArrayView<ref_ptr<WaitHandle>> wait_handles,
      std::chrono::milliseconds timeout = kInfiniteTimeout);

  virtual ~Thread();

  // Returns a process-unique ID for the thread.
  virtual uintptr_t thread_id() = 0;

  // Returns true if this thread is the current thread.
  // Slightly more efficient than thread == Thread::current_thread().
  virtual bool is_current() const = 0;

  // Returns the current thread priority.
  virtual PriorityClass priority_class() const = 0;
  // Sets the priority class of the current thread.
  virtual void set_priority_class(PriorityClass priority_class) = 0;

  // Returns the current processor affinity mask for the thread.
  // The mask is a bit vector in which each bit represents a logical processor
  // that a thread is allowed to run on. The mask is a subset of the process
  // affinity mask for the current process as defined by process_affinity_mask.
  //
  // Compatibility warning: Apple/darwin only support affinity groups, with each
  // unique affinity_mask sharing time. This means that trying to get clever
  // with several thread sets with overlapping masks will likely not work as
  // expected. Try to stick with threads that run only on a single processor.
  virtual uint64_t affinity_mask() const = 0;
  // Sets the processor affinity mask for the thread.
  virtual void set_affinity_mask(uint64_t affinity_mask) = 0;

  // Resumes the thread if it was created suspended.
  // This has no effect if the thread is not suspended.
  virtual void Resume() = 0;

  // Joins with the thread, blocking until it has exited.
  // Returns true if the join was successful or false if the thread is still
  // running.
  //
  // This is equivalent to:
  //   Wait(thread).
  // If you need more flexibility (timeouts, etc) you can pass the thread to a
  // wait routine.
  bool Join() { return Wait(ref_ptr<Thread>(this)) == WaitResult::kSuccess; }

  // Registers a function that will be called when the thread is exiting.
  // Callbacks will be made in reverse order of registration.
  // Note that if the thread is forcefully terminated the callbacks will not
  // be called.
  void RegisterExitCallback(std::function<void()> callback);

 protected:
  Thread();

  // Thread local storage support routines. Used by LocalStorageSlot.
  static uintptr_t AllocateLocalStorageSlot(void (*release_callback)(void*));
  static void DeallocateLocalStorageSlot(uintptr_t slot_id);
  static void* GetLocalStorageSlotValue(uintptr_t slot_id);
  static void SetLocalStorageSlotValue(uintptr_t slot_id, void* value);

  // Called by subclasses when the thread is entered.
  virtual void OnEnter();
  // Called by subclasses when the thread is exiting.
  virtual void OnExit();

  // Name as specified by set_name, if any.
  std::string name_;

  std::mutex exit_mutex_;
  // A list of all registered exit callbacks in the order they were registered.
  // They will be called in reverse order of registration.
  std::vector<std::function<void()>> exit_callbacks_;
};

std::ostream& operator<<(std::ostream& stream, const Thread::WaitResult& value);

}  // namespace xrtl

#endif  // XRTL_BASE_THREADING_THREAD_H_
