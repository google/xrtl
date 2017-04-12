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

#include "xrtl/base/threading/thread.h"

#include "xrtl/base/stopwatch.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/semaphore.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

class ThreadTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    // We call this right away as most of the tests require it and this is the
    // only place we can be sure runs deterministically.
    auto thread = Thread::current_thread();
    EXPECT_NE(nullptr, thread);

    Process::EnableHighResolutionTiming();
  }
  static void TearDownTestCase() { Process::DisableHighResolutionTiming(); }
};

// Tests the logical processor count, mainly for code coverage.
TEST_F(ThreadTest, ProcessorCount) {
  EXPECT_LE(1, Process::logical_processor_count());
}

// Tests high-resolution timing does something.
TEST_F(ThreadTest, HighResolutionTiming) {
  // Eh, we can't actually know it does something. Let's just try turning it on
  // and making sure a sleep is about right.
  Process::EnableHighResolutionTiming();
  Stopwatch stopwatch;
  Thread::Sleep(std::chrono::milliseconds(1));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(5));
  Process::DisableHighResolutionTiming();
}

void TestThreadStartRoutine(void* data) {
  auto fence_event = reinterpret_cast<Event*>(data);
  fence_event->Set();
}

// Tests thread startup.
// Note that this is not exhaustive in the create params (like stack size), as
// those are really platform-specific.
// The create_suspended flag is tested below in the suspend/resume tests.
TEST_F(ThreadTest, Create) {
  // Create an std::function thread and wait for it to run.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() { fence_event->Set(); });
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(thread));

  // Create a function pointer thread and wait for it to run.
  thread = Thread::Create({}, TestThreadStartRoutine, fence_event.get());
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(thread));

  // Tests that some of the create params make it into the thread.
  Thread::CreateParams create_params;
  create_params.name = "SomeName";
  create_params.priority_class = Thread::PriorityClass::kHigh;
  create_params.affinity_mask = 0b1;
  thread = Thread::Create(create_params, [&]() {
    auto self_thread = Thread::current_thread();
    EXPECT_TRUE(self_thread->is_current());
    EXPECT_EQ(create_params.name, Thread::name());
    EXPECT_EQ(create_params.priority_class, self_thread->priority_class());
    EXPECT_EQ(create_params.affinity_mask, self_thread->affinity_mask());
  });
  EXPECT_FALSE(thread->is_current());
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(thread));
}

// Tests threads with a raw function pointer instead of std::function.
TEST_F(ThreadTest, CreateRawFunctionPtr) {
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({},
                               [](void* data) {
                                 auto fence_event =
                                     reinterpret_cast<Event*>(data);
                                 fence_event->Set();
                               },
                               fence_event.get());
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(thread));
}

// Tests thread naming. This is a debug-only feature (really), so it doesn't
// have to be great.
TEST_F(ThreadTest, ThreadName) {
  // Should have a default name.
  EXPECT_FALSE(Thread::name().empty());

  // Reset name to something new.
  std::string original_name = Thread::name();
  std::string new_name = "ThreadTest.ThreadName";
  Thread::set_name(new_name);
  EXPECT_EQ(new_name, Thread::name());
  Thread::set_name(original_name);
}

// Tests nothing.
TEST_F(ThreadTest, TryYield) {
  // Yield may do nothing on some platforms so there's no great way to test it.
  Thread::TryYield();
}

// Tests that Sleep takes some time.
TEST_F(ThreadTest, Sleep) {
  // Sleep(0) should at least yield the thread.
  // TODO(benvanik): spin up a high-pri thread to ensure a context switch
  //                 happens - this could be a bit flaky.
  Stopwatch stopwatch;
  Thread::Sleep(std::chrono::milliseconds(0));
  EXPECT_LT(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Ensure the sleep time is within the right ballpark. We expect it to not
  // be exact, but it's gotta at least be sane.
  stopwatch.Reset();
  Thread::Sleep(std::chrono::milliseconds(50));
  EXPECT_GT(stopwatch.elapsed_micros(), std::chrono::milliseconds(25));
  EXPECT_LT(stopwatch.elapsed_micros(), std::chrono::milliseconds(75));
}

// Tests the TryWait helper.
TEST_F(ThreadTest, TryWait) {
  auto unsignaled_event = Event::CreateManualResetEvent(false);
  auto signaled_event = Event::CreateManualResetEvent(true);
  EXPECT_FALSE(Thread::TryWait(unsignaled_event));
  EXPECT_TRUE(Thread::TryWait(signaled_event));
}

// Tests mixing and matching wait handle types.
// NOTE: the wait methods are mostly tested within the various wait handle tests
//       (such as event_test.cc). These tests are specifically for testing
//       threads as wait handles.
TEST_F(ThreadTest, MixedWaitHandles) {
  auto unsignaled_event = Event::CreateManualResetEvent(false);
  auto signaled_event = Event::CreateManualResetEvent(true);
  auto unavailable_semaphore = Semaphore::Create(0, 100);
  auto available_semaphore = Semaphore::Create(100, 100);

  // WaitAny with the various modes.
  auto results = Thread::WaitAny({signaled_event, available_semaphore});
  EXPECT_EQ(Thread::WaitResult::kSuccess, results.wait_result);
  results = Thread::WaitAny({unsignaled_event, unavailable_semaphore},
                            kImmediateTimeout);
  EXPECT_EQ(Thread::WaitResult::kTimeout, results.wait_result);
  results = Thread::WaitAny({unsignaled_event, available_semaphore});
  EXPECT_EQ(Thread::WaitResult::kSuccess, results.wait_result);
  EXPECT_EQ(1, results.wait_handle_index);

  // WaitAll with the various modes.
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({signaled_event, available_semaphore}));
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({unsignaled_event, unavailable_semaphore},
                            kImmediateTimeout));
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({signaled_event, unavailable_semaphore},
                            kImmediateTimeout));
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({unsignaled_event, available_semaphore},
                            kImmediateTimeout));
}

// Tests that thread_id is something valid.
TEST_F(ThreadTest, ThreadId) {
  // Check self ID as something valid.
  uintptr_t main_thread_id = Thread::current_thread()->thread_id();
  EXPECT_NE(0, main_thread_id);
  EXPECT_TRUE(Thread::current_thread()->is_current());

  // Create a new thread and ensure it gets a different ID.
  uintptr_t child_thread_id = 0;
  Thread::CreateParams create_params;
  create_params.create_suspended = true;
  ref_ptr<Thread> thread;
  thread = Thread::Create(create_params, [&]() {
    EXPECT_TRUE(thread->is_current());
    EXPECT_NE(main_thread_id, Thread::current_thread()->thread_id());
    EXPECT_EQ(child_thread_id, Thread::current_thread()->thread_id());
  });
  child_thread_id = thread->thread_id();
  EXPECT_NE(main_thread_id, child_thread_id);
  EXPECT_FALSE(thread->is_current());
  thread->Resume();
  EXPECT_TRUE(thread->Join());
}

// Tests setting thread priority classes.
TEST_F(ThreadTest, PriorityClass) {
  // TODO(benvanik): a good way to test priority had an effect. Could spin
  //                 low and high priority threads and see which one gets more
  //                 time slices.
  auto thread = Thread::current_thread();
  auto original_priority_class = thread->priority_class();
  thread->set_priority_class(Thread::PriorityClass::kHigh);
  EXPECT_EQ(Thread::PriorityClass::kHigh, thread->priority_class());
  thread->set_priority_class(original_priority_class);
  EXPECT_EQ(original_priority_class, thread->priority_class());
}

// Tests setting thread affinity masks.
TEST_F(ThreadTest, AffinityMask) {
  // TODO(benvanik): a good way to test affinity masks. On cpu>=2 systems could
  //                 spin up two threads and make sure they both get roughly
  //                 equivalent times when run on their own threads.
  auto thread = Thread::current_thread();
  auto original_affinity_mask = thread->affinity_mask();
  thread->set_affinity_mask(0b1);
  EXPECT_EQ(0b1, thread->affinity_mask());
  thread->set_affinity_mask(original_affinity_mask);
  EXPECT_EQ(original_affinity_mask, thread->affinity_mask());
}

// Tests creating a thread suspended.
TEST_F(ThreadTest, CreateSuspended) {
  // Create thread suspended.
  Thread::CreateParams create_params;
  create_params.create_suspended = true;
  std::atomic<bool> has_started_thread{false};
  auto thread =
      Thread::Create(create_params, [&]() { has_started_thread = true; });

  // Should not start the thread, so we can sleep for a sec.
  EXPECT_FALSE(has_started_thread);
  Thread::Sleep(std::chrono::milliseconds(50));
  EXPECT_FALSE(has_started_thread);

  // Resume the thread.
  thread->Resume();

  // Resuming again should be a no-op.
  thread->Resume();

  // Wait for the thread to exit and ensure our routine actually ran.
  EXPECT_TRUE(thread->Join());
  EXPECT_TRUE(has_started_thread);

  // Resuming on a zombie thread should be a no-op.
  thread->Resume();
}

// Tests joining a thread via the simple Join method.
TEST_F(ThreadTest, Join) {
  // Create thread.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait before exiting so the main thread has some control over the order.
    EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
    // Sleep for a bit to keep the main thread in suspense >_<
    Thread::Sleep(std::chrono::milliseconds(50));
  });
  fence_event->Set();
  EXPECT_TRUE(thread->Join());
  // No-op on already exited thread.
  EXPECT_TRUE(thread->Join());
}

// Tests joining a thread by waiting.
TEST_F(ThreadTest, JoinWait) {
  // Create thread.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait before exiting so the main thread has some control over the order.
    EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
    // Sleep so that our timeout has some actual delay.
    Thread::Sleep(std::chrono::milliseconds(100));
  });

  // Joining now should time out, as the thread is still running.
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::Wait(thread, kImmediateTimeout));

  // Timeout with a duration. We should fully timeout.
  Stopwatch stopwatch;
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::Wait(thread, std::chrono::milliseconds(50)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(25));

  // Poke the thread to run. It'll wait for a bit before exiting, which we can
  // use to test our timeout exits early.
  fence_event->Set();
  stopwatch.Reset();
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(thread));
  EXPECT_GT(stopwatch.elapsed_micros(), std::chrono::milliseconds(50));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(200));

  // Joining an already exited thread should be immediate.
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(thread));
}

}  // namespace
}  // namespace xrtl
