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

#ifndef XRTL_BASE_THREADING_MESSAGE_LOOP_H_
#define XRTL_BASE_THREADING_MESSAGE_LOOP_H_

#include <chrono>
#include <functional>
#include <mutex>
#include <vector>

#include "xrtl/base/intrusive_list.h"
#include "xrtl/base/intrusive_pool.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/base/threading/wait_handle.h"

namespace xrtl {

// Asynchronous platform message loop.
// Message loops are used primarily for UI work with this type mapping to the
// underlying platform primitive (such as Looper, Win32 message loops, or
// CFRunLoop, etc). They exist as a thread that can execute functions and may
// occasionally make calls out based on user events.
//
// Message loops are not designed for high performance and should not be used
// in place of a proper task queue. The underlying system primitives on which
// the loop is based may have significant per-message overhead (hundreds or more
// microseconds/message).
//
// Loops must be exited with Exit; simply dropping all references will likely
// leak. Though not required the wait handle should also be used to ensure all
// loop tasks have completed before proceeding with any teardown logic.
//
// Usage:
//  auto loop = MessageLoop::Create();
//  loop->MarshalSync([]() {
//    // Runs synchronously on the loop thread, blocking the calling thread
//    // until it returns.
//  });
//  TaskList task_list;
//  loop->MarshalAsync(&task_list, []() {
//    // Runs asynchronously at some point in the future on the loop thread.
//  });
//  OnButtonPressOrWhatever([]() {
//    loop->Exit();  // Start shutting down the loop.
//  });
//  Thread::Wait(loop);  // Wait until the loop exits.
class MessageLoop : public WaitHandle {
 public:
  class Task;
  class TaskList;

  // Creates a new message loop thread.
  static ref_ptr<MessageLoop> Create();

  virtual ~MessageLoop();

  // Returns true if the currently executing code is running on the loop thread.
  virtual bool is_loop_thread() = 0;

  // Marshals a callback onto the message loop thread synchronously.
  // The callback will be executed on the thread in FIFO order with all other
  // tasks before this call returns.
  //
  // It's rarely a good idea to use blocking calls like this; if the loop is
  // running large tasks (10-20ms/etc) the calling thread will block for that
  // amount of time (or potentially longer if there's a large queue).
  //
  // This is safe to call from the loop thread (the callback will just be made
  // inline), but that's probably not what you want.
  virtual void MarshalSync(std::function<void()> callback) = 0;

  // Marshals a callback onto the message loop thread asynchronously.
  // The callback will be queued for execution on the thread in FIFO order with
  // all other tasks and this call will return immediately.
  //
  // The task_list is used to safely cancel the callback and when the
  // pending_task_list is destroyed all callbacks queued on it will be
  // canceled. See TaskList for more information.
  //
  // Usage:
  //  TaskList task_list;  // Can be kept as a class member, etc.
  //  loop->MarshalAsync(&task_list, []() {
  //    // Running on the loop thread.
  //  });
  virtual void MarshalAsync(TaskList* pending_task_list,
                            std::function<void()> callback);

  // Defers the function call until the next thread tick.
  // This ensures that the callback will not run in the same call stack and can
  // be treated as if it was asynchronous.
  // The callback will not be executed if the loop exits before it is run or
  // the TaskList is deallocated.
  ref_ptr<Task> Defer(TaskList* pending_task_list,
                      std::function<void()> callback);

  // Defers the function call until the given delay elapses.
  // The exact timing of the callback is undefined and it may occur earlier or
  // later than the requested delay time.
  // The callback will not be executed if the loop exits before it is run or
  // the TaskList is deallocated.
  ref_ptr<Task> Defer(TaskList* pending_task_list,
                      std::function<void()> callback,
                      std::chrono::milliseconds delay_millis);

  // Defers the function call and repeatedly calls it until canceled.
  // The exact timing of the callback is undefined and it may occur earlier or
  // later than the requested delay time.
  // The callback will not be executed if the loop exits before it is run or
  // the TaskList is deallocated.
  ref_ptr<Task> DeferRepeating(TaskList* pending_task_list,
                               std::function<void()> callback,
                               std::chrono::milliseconds period_millis);

  // Defers the function call until the given delay elapses and repeatedly calls
  // it until it is canceled.
  // The exact timing of the callback is undefined and it may occur earlier or
  // later than the requested delay time.
  // The callback will not be executed if the loop exits before it is run or
  // the TaskList is deallocated.
  ref_ptr<Task> DeferRepeating(TaskList* pending_task_list,
                               std::function<void()> callback,
                               std::chrono::milliseconds delay_millis,
                               std::chrono::milliseconds period_millis);

  // Registers a function that will be called when the loop is exiting.
  // Callbacks will be made in reverse order of registration.
  // The callbacks will run on the loop thread.
  void RegisterExitCallback(std::function<void()> callback);

  // Requests the loop exit.
  // This may be called from any thread.
  // The message loop exit_wait_handle can be waited on à la Thread to join with
  // it once it has completed shutdown.
  // Returns a wait handle that can be used to wait for loop exit.
  virtual ref_ptr<WaitHandle> Exit() = 0;

  // An asynchronous task that can be queued for execution on a message loop.
  // Tasks may be executed in the following tick (delay = 0, repeat = 0),
  // delayed and one-shot (delay = N, repeat = 0), or repeating until aborted
  // (delay = 0-N, repeat = M).
  //
  // Tasks are reference counted and kept alive while scheduled for execution.
  // Tasks are descheduled after they complete and perform their callback or
  // when they are canceled before they can complete. Users can retain the task
  // to cancel it early if desired, but should always release their reference
  // ASAP to ensure tasks are pooled effectively.
  //
  // Any user of tasks must retain a TaskList. This allows for
  // cancellation of all pending tasks when the list is deallocated, preventing
  // use-after-free by tasks that complete after the code that spawned them is
  // gone.
  class Task : public RefObject<Task> {
   public:
    virtual ~Task();

    // The message loop the task is assigned to run on.
    MessageLoop* message_loop() const { return message_loop_; }

    // Internal. Do not use.
    // TODO(benvanik): hide better.
    uintptr_t platform_handle() const { return platform_handle_; }
    void set_platform_handle(uintptr_t platform_handle) {
      platform_handle_ = platform_handle;
    }

    // The delay before the task is first invoked, in milliseconds.
    // If 0 the task will be executed ASAP.
    std::chrono::milliseconds delay_millis() const { return delay_millis_; }
    // The interval between repetitions (after the first invocation).
    // If 0 the task will not be repeated.
    std::chrono::milliseconds period_millis() const { return period_millis_; }

    // Cancels the task, descheduling it and preventing any future callbacks.
    // The callback will not be called. This may be called from any thread.
    void Cancel();

    // Returns the task back to the message loop pool.
    static void Delete(Task* task);

   private:
    template <typename T, size_t kOffset>
    friend class IntrusivePool;
    template <typename T, size_t kOffset>
    friend class IntrusivePoolBase;
    friend class TaskList;
    friend class MessageLoop;
    friend class Win32MessageLoop;

    // Created and pooled by MessageLoop's intrusive pool.
    Task();

    // Initializes the task for use.
    void Initialize(MessageLoop* message_loop, TaskList* pending_task_list,
                    std::chrono::milliseconds delay_millis,
                    std::chrono::milliseconds period_millis,
                    std::function<void()> callback);

    void Invoke();

    // The mutex that guards the invoke/cancel behavior.
    // We make it recursive so that tasks are safe to call Cancel() on
    // themselves (needed for canceling repeating timers, etc).
    std::recursive_mutex mutex_;
    MessageLoop* message_loop_ = nullptr;
    TaskList* pending_task_list_ = nullptr;
    std::function<void()> callback_;
    std::chrono::milliseconds delay_millis_{0};
    std::chrono::milliseconds period_millis_{0};
    bool is_alive_ = true;

    // Varies based on platform; on web this is the browser timer handle.
    uintptr_t platform_handle_ = 0;

    // Presence in the TaskList.
    IntrusiveListLink pending_task_list_link_;
    // Allows the thread to track the task while scheduled.
    IntrusiveListLink loop_list_link_;
  };

  // Utility that tracks all pending tasks and cancels them when it is
  // deallocated. This prevents use-after-frees that happen when tasks outlive
  // the code that allocated them.
  //
  // Any code using tasks must use a task list. The easiest way is to make the
  // TaskList a member so that it is automatically deallocated with the
  // containing type.
  //
  // NOTE: it should always be the first private member of the type. This
  //       ensures it is constructed first and destructed last.
  //
  // Usage:
  //   class MyType {
  //    public:
  //     void DoSomething() {
  //       loop->Defer(&pending_task_list_, []() {});
  //     }
  //    private:
  //     TaskList pending_task_list_;
  //     // other members here
  //   };
  class TaskList {
   public:
    TaskList();
    ~TaskList();

   private:
    friend class Task;

    void RegisterTask(Task* task);
    void UnregisterTask(Task* task);

    // Tasks may be registered/unregistered cross-thread and this guards the
    // list.
    std::mutex mutex_;
    // Weak list of all tasks registered with this task list, unretained.
    // Tasks register and unregister themselves as they are allocated/deleted.
    IntrusiveList<Task, offsetof(Task, pending_task_list_link_)> list_;
  };

 protected:
  friend class Task;
  friend class CFRunLoopMessageLoop;

  MessageLoop();

  // Called by subclasses when the loop is started.
  void OnEnter();
  // Called by subclasses when the loop is exited.
  void OnExit();

  ref_ptr<Task> DeferTask(TaskList* pending_task_list,
                          std::function<void()> callback,
                          std::chrono::milliseconds delay_millis,
                          std::chrono::milliseconds period_millis);

  // Schedules an async task for future execution.
  // This is called once when the task is first prepared.
  // Subclasses can use this to trigger platform timers, reprioritize run
  // queues, etc.
  virtual void ScheduleTask(ref_ptr<Task> task);
  // Deschedules a previously-scheduled async task.
  // This is called once when the task is canceled or after it has completed.
  // Subclasses can use this to clean up platform resources.
  virtual void DescheduleTask(ref_ptr<Task> task);
  // Returns true if the given task is scheduled for future callback.
  bool IsTaskScheduled(Task* task);
  // Invokes the given task.
  void InvokeTask(ref_ptr<Task> task);
  // Deallocates an async task, returning it to the loop pool.
  virtual void DeallocateTask(Task* task);

  // A mutex that must be held when manipulating the task data structures as we
  // may manipulate them from multiple threads.
  std::recursive_mutex task_mutex_;
  // Pool of tasks recycled across the various Defer'ed calls.
  // As defers can be high frequency this keeps us from creating too much
  // garbage.
  IntrusivePool<Task, offsetof(Task, loop_list_link_)> task_pool_{32, 128};
  // All currently scheduled tasks. Only tasks pending a callback are present in
  // this list. They are retained until canceled or completed.
  IntrusiveList<ref_ptr<Task>, offsetof(Task, loop_list_link_)> pending_tasks_;

  // Set true when we start exiting to prevent posting more work.
  std::atomic<bool> exiting_{false};
  // A list of all registered exit callbacks in the order they were registered.
  // They will be called in reverse order of registration.
  std::vector<std::function<void()>> exit_callbacks_;
};

}  // namespace xrtl

#endif  // XRTL_BASE_THREADING_MESSAGE_LOOP_H_
