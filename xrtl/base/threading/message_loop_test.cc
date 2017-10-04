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

#include "xrtl/base/threading/message_loop.h"

#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

// TSAN is very particular about our lambda captures of tasks.
// We know they are ok (as we are fencing ourselves), but this is required to
// make it happy.
template <typename T>
class SafePtr {
 public:
  T* value() {
    Thread::Wait(ready_fence_);
    return value_.load();
  }
  void set_value(T* value) {
    value_ = value;
    ready_fence_->Set();
  }

 private:
  ref_ptr<Event> ready_fence_ = Event::CreateFence();
  std::atomic<T*> value_{nullptr};
};

using SafeLoopPtr = SafePtr<MessageLoop>;
using SafeTaskPtr = SafePtr<MessageLoop::Task>;

// Tests initialization of the message loop.
TEST(MessageLoopTest, Initialization) {
  // Create and immediately delete the loop.
  auto loop = MessageLoop::Create();
  EXPECT_FALSE(loop->is_loop_thread());
  Thread::Wait(loop->Exit());
}

// Tests exit callbacks.
TEST(MessageLoopTest, ExitCallback) {
  // Test registering from outside the loop.
  auto loop = MessageLoop::Create();
  EXPECT_FALSE(loop->is_loop_thread());
  bool did_call = false;
  loop->RegisterExitCallback([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    EXPECT_FALSE(did_call);
    did_call = true;
  });
  Thread::Wait(loop->Exit());
  EXPECT_TRUE(did_call);

  // Test registering from inside the loop.
  loop = MessageLoop::Create();
  EXPECT_FALSE(loop->is_loop_thread());
  did_call = false;
  loop->MarshalSync([&]() {
    loop->RegisterExitCallback([&]() {
      EXPECT_TRUE(loop->is_loop_thread());
      EXPECT_FALSE(did_call);
      did_call = true;
    });
  });
  Thread::Wait(loop->Exit());
  EXPECT_TRUE(did_call);
}

// Tests the exit callback ordering (reverse of registration).
TEST(MessageLoopTest, ExitCallbackOrdering) {
  auto loop = MessageLoop::Create();
  EXPECT_FALSE(loop->is_loop_thread());
  bool did_call_1 = false;
  bool did_call_2 = false;
  bool did_call_3 = false;
  loop->RegisterExitCallback([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    EXPECT_TRUE(did_call_1);
    EXPECT_TRUE(did_call_2);
    EXPECT_FALSE(did_call_3);
    did_call_3 = true;
  });
  loop->RegisterExitCallback([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    EXPECT_TRUE(did_call_1);
    EXPECT_FALSE(did_call_2);
    EXPECT_FALSE(did_call_3);
    did_call_2 = true;
  });
  loop->RegisterExitCallback([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    EXPECT_FALSE(did_call_1);
    EXPECT_FALSE(did_call_2);
    EXPECT_FALSE(did_call_3);
    did_call_1 = true;
  });
  Thread::Wait(loop->Exit());
  EXPECT_TRUE(did_call_1);
  EXPECT_TRUE(did_call_2);
  EXPECT_TRUE(did_call_3);
}

// Tests implicit task cancellation when tasks are queued in exit callbacks.
// This will emit warnings, but should work.
TEST(MessageLoopTest, ExitCallbackTaskCancellation) {
  auto loop = MessageLoop::Create();
  EXPECT_FALSE(loop->is_loop_thread());
  bool did_call = false;
  bool did_make_async_call = false;
  MessageLoop::TaskList task_list;
  loop->RegisterExitCallback([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    EXPECT_FALSE(did_call);
    did_call = true;

    loop->MarshalAsync(&task_list, [&]() {
      EXPECT_FALSE(did_make_async_call);
      did_make_async_call = true;
    });
  });
  Thread::Wait(loop->Exit());
  EXPECT_TRUE(did_call);
  EXPECT_FALSE(did_make_async_call);
}

// Tests MarshalSync.
TEST(MessageLoopTest, MarshalSync) {
  auto loop = MessageLoop::Create();

  // Marshal a call onto the loop.
  bool did_complete = false;
  EXPECT_FALSE(loop->is_loop_thread());
  loop->MarshalSync([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    EXPECT_FALSE(did_complete);
    did_complete = true;
  });
  EXPECT_TRUE(did_complete);

  Thread::Wait(loop->Exit());
}

// Tests that MarshalSync is re-entrant.
TEST(MessageLoopTest, MarshalSyncReentrant) {
  auto loop = MessageLoop::Create();

  bool did_complete = false;
  loop->MarshalSync([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    bool did_complete_reentrant = false;
    loop->MarshalSync([&]() {
      EXPECT_TRUE(loop->is_loop_thread());
      EXPECT_FALSE(did_complete_reentrant);
      did_complete_reentrant = true;
    });
    EXPECT_TRUE(did_complete_reentrant);
    EXPECT_FALSE(did_complete);
    did_complete = true;
  });
  EXPECT_TRUE(did_complete);

  Thread::Wait(loop->Exit());
}

// Tests exiting from within a MarshalSync callback.
TEST(MessageLoopTest, MarshalSyncExit) {
  auto loop = MessageLoop::Create();
  loop->MarshalSync([&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    loop->Exit();
  });
  Thread::Wait(loop);
}

// Tests MarshalAsync.
TEST(MessageLoopTest, MarshalAsync) {
  auto loop = MessageLoop::Create();

  auto fence_event = Event::CreateFence();
  bool did_complete = false;
  MessageLoop::TaskList task_list;
  loop->MarshalAsync(&task_list, [&]() {
    EXPECT_FALSE(did_complete);
    did_complete = true;
    fence_event->Set();
  });

  Thread::Wait(fence_event);

  EXPECT_TRUE(did_complete);

  Thread::Wait(loop->Exit());
}

// Tests that MarshalAsync is re-entrant.
TEST(MessageLoopTest, MarshalAsyncReentrant) {
  auto loop = MessageLoop::Create();

  auto fence_event = Event::CreateFence();
  bool did_complete = false;
  bool did_complete_reentrant = false;
  MessageLoop::TaskList task_list;
  loop->MarshalAsync(&task_list, [&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    loop->MarshalAsync(&task_list, [&]() {
      EXPECT_TRUE(loop->is_loop_thread());
      EXPECT_FALSE(did_complete_reentrant);
      did_complete_reentrant = true;
      fence_event->Set();
    });
    EXPECT_FALSE(did_complete);
    did_complete = true;
  });

  Thread::Wait(fence_event);
  Thread::Wait(loop->Exit());

  EXPECT_TRUE(did_complete);
  EXPECT_TRUE(did_complete_reentrant);
}

// Tests MarshalAsync calls canceling themselves.
TEST(MessageLoopTest, MarshalAsyncCancelReentrant) {
  auto loop = MessageLoop::Create();

  // We queue up a task that we won't be canceling that waits until we've
  // queued up the next task. That way when we cancel we can be sure of the
  // timing.
  auto fence_event = Event::CreateFence();
  auto task_list = absl::make_unique<MessageLoop::TaskList>();
  loop->MarshalAsync(task_list.get(), [&]() {
    // Cancel ourselves.
    task_list.reset();
    fence_event->Set();
  });

  // Wait for the task to complete.
  Thread::Wait(fence_event);

  // Join the loop thread (so we are 100% sure its done).
  Thread::Wait(loop->Exit());
}

// Tests canceling MarshalAsync calls with the task list from the loop thread.
TEST(MessageLoopTest, MarshalAsyncCancelInLoop) {
  auto loop = MessageLoop::Create();

  // We queue up a task that we won't be canceling that waits until we've
  // queued up the next task. That way when we cancel we can be sure of the
  // timing.
  auto fence_event = Event::CreateFence();
  MessageLoop::TaskList persistent_task_list;
  auto task_list = absl::make_unique<MessageLoop::TaskList>();
  loop->MarshalAsync(&persistent_task_list, [&]() {
    // Wait in the loop thread.
    Thread::Wait(fence_event);

    // Cancel the task.
    task_list.reset();
  });

  // Queue up the task we will cancel.
  bool did_task_run = false;
  loop->MarshalAsync(task_list.get(), [&]() {
    EXPECT_FALSE(did_task_run);
    did_task_run = true;
  });

  // Unblock the loop thread.
  fence_event->Set();

  // Join the loop thread (so we are 100% sure its done).
  Thread::Wait(loop->Exit());

  // Shouldn't have run the task.
  EXPECT_FALSE(did_task_run);
}

// Tests canceling MarshalAsync calls with the task list from another thread.
TEST(MessageLoopTest, MarshalAsyncCancelOffThread) {
  auto loop = MessageLoop::Create();

  // We queue up a task that we won't be canceling that waits until we've
  // queued up the next task. That way when we cancel we can be sure of the
  // timing.
  auto fence_event = Event::CreateFence();
  MessageLoop::TaskList persistent_task_list;
  loop->MarshalAsync(&persistent_task_list, [&]() {
    // Wait in the loop thread.
    Thread::Wait(fence_event);
  });

  // Queue up the task we will cancel.
  bool did_task_run = false;
  auto task_list = absl::make_unique<MessageLoop::TaskList>();
  loop->MarshalAsync(task_list.get(), [&]() {
    EXPECT_FALSE(did_task_run);
    did_task_run = true;
  });

  // Cancel the task.
  task_list.reset();

  // Unblock the loop thread.
  fence_event->Set();

  // Join the loop thread (so we are 100% sure its done).
  Thread::Wait(loop->Exit());

  // Shouldn't have run the task.
  EXPECT_FALSE(did_task_run);
}

// Tests exiting from within a MarshalAsync callback.
TEST(MessageLoopTest, MarshalAsyncExit) {
  auto loop = MessageLoop::Create();
  MessageLoop::TaskList task_list;
  loop->MarshalAsync(&task_list, [&]() {
    EXPECT_TRUE(loop->is_loop_thread());
    loop->Exit();
  });
  Thread::Wait(loop);
}

// Tests the Defer task method.
TEST(MessageLoopTest, Defer) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  bool did_run = false;
  MessageLoop::TaskList task_list;
  loop->Defer(&task_list, [&]() {
    EXPECT_FALSE(did_run);
    did_run = true;
    fence_event->Set();
  });
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  Thread::Wait(loop->Exit());
}

// Tests canceling a deferred task explicitly via Cancel.
TEST(MessageLoopTest, DeferCancelExplicit) {
  auto loop = MessageLoop::Create();

  // We queue up a task that we won't be canceling that waits until we've
  // queued up the next task. That way when we cancel we can be sure of the
  // timing.
  auto fence_event = Event::CreateFence();
  MessageLoop::TaskList persistent_task_list;
  loop->MarshalAsync(&persistent_task_list, [&]() {
    // Wait in the loop thread.
    Thread::Wait(fence_event);
  });

  // Queue up the task.
  bool did_run = false;
  MessageLoop::TaskList task_list;
  auto task = loop->Defer(&task_list, [&]() {
    EXPECT_FALSE(did_run);
    did_run = true;
  });

  // Cancel the task, which should prevent it from running.
  task->Cancel();

  // Resume the loop thread.
  fence_event->Set();
  Thread::Wait(loop->Exit());
  EXPECT_FALSE(did_run);
}

// Tests canceling a deferred task implicitly via TaskList.
TEST(MessageLoopTest, DeferCancelImplicit) {
  auto loop = MessageLoop::Create();

  // We queue up a task that we won't be canceling that waits until we've
  // queued up the next task. That way when we cancel we can be sure of the
  // timing.
  auto fence_event = Event::CreateFence();
  MessageLoop::TaskList persistent_task_list;
  loop->MarshalAsync(&persistent_task_list, [&]() {
    // Wait in the loop thread.
    Thread::Wait(fence_event);
  });

  // Queue up the task.
  bool did_run = false;
  auto task_list = absl::make_unique<MessageLoop::TaskList>();
  loop->Defer(task_list.get(), [&]() {
    EXPECT_FALSE(did_run);
    did_run = true;
  });

  // Cancel the task via the list, which should prevent it from running.
  task_list.reset();

  // Resume the loop thread.
  fence_event->Set();
  Thread::Wait(loop->Exit());
  EXPECT_FALSE(did_run);
}

// Tests canceling a deferred task after it has run.
TEST(MessageLoopTest, DeferCancelNop) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  bool did_run = false;
  MessageLoop::TaskList task_list;
  auto task = loop->Defer(&task_list, [&]() {
    EXPECT_FALSE(did_run);
    did_run = true;
    fence_event->Set();
  });
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  task->Cancel();
  Thread::Wait(loop->Exit());
}

// Tests canceling a deferred task from within itself.
TEST(MessageLoopTest, DeferCancelReentrant) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  MessageLoop::TaskList task_list;
  SafeTaskPtr task_ptr;
  bool did_run = false;
  auto task = loop->Defer(&task_list, [&]() {
    EXPECT_FALSE(did_run);
    did_run = true;
    task_ptr.value()->Cancel();
    fence_event->Set();
  });
  task_ptr.set_value(task.get());
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  Thread::Wait(loop->Exit());
}

// Tests the delayed Defer task method.
TEST(MessageLoopTest, DelayedDefer) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  bool did_run = false;
  MessageLoop::TaskList task_list;
  loop->Defer(&task_list,
              [&]() {
                EXPECT_FALSE(did_run);
                did_run = true;
                fence_event->Set();
              },
              std::chrono::milliseconds(5));
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  Thread::Wait(loop->Exit());
}

// Tests canceling a delayed deferred task explicitly via Cancel.
TEST(MessageLoopTest, DelayedDeferCancelExplicit) {
  auto loop = MessageLoop::Create();

  // Queue up the task.
  bool did_run = false;
  MessageLoop::TaskList task_list;
  auto task = loop->Defer(&task_list,
                          [&]() {
                            EXPECT_FALSE(did_run);
                            did_run = true;
                          },
                          std::chrono::seconds(100));

  // Cancel the task, which should prevent it from running.
  task->Cancel();

  Thread::Wait(loop->Exit());
  EXPECT_FALSE(did_run);
}

// Tests canceling a delayed deferred task implicitly via TaskList.
TEST(MessageLoopTest, DelayedDeferCancelImplicit) {
  auto loop = MessageLoop::Create();

  // Queue up the task.
  bool did_run = false;
  auto task_list = absl::make_unique<MessageLoop::TaskList>();
  loop->Defer(task_list.get(),
              [&]() {
                EXPECT_FALSE(did_run);
                did_run = true;
              },
              std::chrono::seconds(100));

  // Cancel the task via the list, which should prevent it from running.
  task_list.reset();

  Thread::Wait(loop->Exit());
  EXPECT_FALSE(did_run);
}

// Tests canceling a delayed deferred task after it has run.
TEST(MessageLoopTest, DelayedDeferCancelNop) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  bool did_run = false;
  MessageLoop::TaskList task_list;
  auto task = loop->Defer(&task_list,
                          [&]() {
                            EXPECT_FALSE(did_run);
                            did_run = true;
                            fence_event->Set();
                          },
                          std::chrono::milliseconds(10));
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  task->Cancel();
  Thread::Wait(loop->Exit());
}

// Tests canceling a delayed deferred task from within itself.
TEST(MessageLoopTest, DelayedDeferCancelReentrant) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  bool did_run = false;
  MessageLoop::TaskList task_list;
  SafeTaskPtr task_ptr;
  auto task = loop->Defer(&task_list,
                          [&]() {
                            EXPECT_FALSE(did_run);
                            did_run = true;
                            task_ptr.value()->Cancel();
                            fence_event->Set();
                          },
                          std::chrono::milliseconds(10));
  task_ptr.set_value(task.get());
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  Thread::Wait(loop->Exit());
}

// Tests the DeferRepeating task method.
TEST(MessageLoopTest, DeferRepeating) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  int run_count = 0;
  MessageLoop::TaskList task_list;
  SafeTaskPtr task_ptr;
  auto task = loop->DeferRepeating(&task_list,
                                   [&]() {
                                     ++run_count;
                                     if (run_count == 5) {
                                       task_ptr.value()->Cancel();
                                       fence_event->Set();
                                     }
                                   },
                                   std::chrono::milliseconds(5),
                                   std::chrono::milliseconds(1));
  task_ptr.set_value(task.get());
  Thread::Wait(fence_event);
  EXPECT_EQ(5, run_count);
  Thread::Wait(loop->Exit());
}

// Tests canceling a repeating deferred task explicitly via Cancel.
TEST(MessageLoopTest, DeferRepeatingCancelExplicit) {
  auto loop = MessageLoop::Create();

  // Queue up the task.
  bool did_run = false;
  MessageLoop::TaskList task_list;
  auto task = loop->DeferRepeating(&task_list,
                                   [&]() {
                                     EXPECT_FALSE(did_run);
                                     did_run = true;
                                   },
                                   std::chrono::seconds(100),
                                   std::chrono::seconds(100));

  // Cancel the task, which should prevent it from running.
  task->Cancel();

  Thread::Wait(loop->Exit());
  EXPECT_FALSE(did_run);
}

// Tests canceling a repeating deferred task implicitly via TaskList.
TEST(MessageLoopTest, DeferRepeatingCancelImplicit) {
  auto loop = MessageLoop::Create();

  // Queue up the task.
  bool did_run = false;
  auto task_list = absl::make_unique<MessageLoop::TaskList>();
  loop->DeferRepeating(task_list.get(),
                       [&]() {
                         EXPECT_FALSE(did_run);
                         did_run = true;
                       },
                       std::chrono::seconds(100), std::chrono::seconds(100));

  // Cancel the task via the list, which should prevent it from running.
  task_list.reset();

  Thread::Wait(loop->Exit());
  EXPECT_FALSE(did_run);
}

// Tests canceling a repeating deferred task after it has run.
TEST(MessageLoopTest, DeferRepeatingCancelNop) {
  auto loop = MessageLoop::Create();
  auto fence_event = Event::CreateFence();
  bool did_run = false;
  MessageLoop::TaskList task_list;
  SafeTaskPtr task_ptr;
  auto task = loop->DeferRepeating(&task_list,
                                   [&]() {
                                     EXPECT_FALSE(did_run);
                                     did_run = true;
                                     task_ptr.value()->Cancel();
                                     fence_event->Set();
                                   },
                                   std::chrono::milliseconds(5),
                                   std::chrono::milliseconds(5));
  task_ptr.set_value(task.get());
  Thread::Wait(fence_event);
  EXPECT_TRUE(did_run);
  task->Cancel();
  Thread::Wait(loop->Exit());
}

}  // namespace
}  // namespace xrtl
