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

#include "xrtl/base/threading/semaphore.h"

#include "xrtl/base/stopwatch.h"
#include "xrtl/base/threading/event.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

bool ShouldBlock(ref_ptr<Semaphore> semaphore) {
  return Thread::Wait(semaphore, kImmediateTimeout) ==
         Thread::WaitResult::kTimeout;
}

bool ShouldNotBlock(ref_ptr<Semaphore> semaphore) {
  return Thread::Wait(semaphore, kImmediateTimeout) ==
         Thread::WaitResult::kSuccess;
}

class SemaphoreTest : public ::testing::Test {
 public:
  // We need high resolution timing to ensure our waits are measured correctly.
  static void SetUpTestCase() { Process::EnableHighResolutionTiming(); }
  static void TearDownTestCase() { Process::DisableHighResolutionTiming(); }
};

// Tests basic semaphore acquire/release usage.
TEST_F(SemaphoreTest, BasicUsage) {
  // Start at initial usage of zero; semaphore should block.
  auto semaphore = Semaphore::Create(0, 2);
  EXPECT_NE(nullptr, semaphore);
  EXPECT_EQ(2, semaphore->maximum_count());
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Release once (count = 1).
  int previous_count = -1;
  EXPECT_TRUE(semaphore->Release(1, &previous_count));
  EXPECT_EQ(0, previous_count);

  // Release again (count = 2).
  EXPECT_TRUE(semaphore->Release(1, &previous_count));
  EXPECT_EQ(1, previous_count);

  // Try to release once more, which should fail because it would go over the
  // maximum_count of 2.
  EXPECT_FALSE(semaphore->Release(1, &previous_count));

  // Acquire once (count = 1).
  EXPECT_TRUE(ShouldNotBlock(semaphore));

  // Acquire again (count = 0).
  EXPECT_TRUE(ShouldNotBlock(semaphore));

  // Try to acquire past 0. Should block.
  EXPECT_TRUE(ShouldBlock(semaphore));
}

// Tests the initial_count creation parameter.
TEST_F(SemaphoreTest, InitialCount) {
  // Start at initial count of 0; should block.
  auto semaphore = Semaphore::Create(0, 2);
  EXPECT_NE(nullptr, semaphore);
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Start at initial count of 1; should be able to acquire 1.
  semaphore = Semaphore::Create(1, 2);
  EXPECT_NE(nullptr, semaphore);
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldBlock(semaphore));
}

// Tests the various forms of Release.
TEST_F(SemaphoreTest, MultiRelease) {
  auto semaphore = Semaphore::Create(0, 100);
  EXPECT_NE(nullptr, semaphore);
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Release one and acquire one.
  EXPECT_TRUE(semaphore->Release());
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Release multiple at the same time.
  EXPECT_TRUE(semaphore->Release(2));
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Use the previous count result.
  int previous_count = -1;
  EXPECT_TRUE(semaphore->Release(1, &previous_count));
  EXPECT_EQ(0, previous_count);
  EXPECT_TRUE(semaphore->Release(2, &previous_count));
  EXPECT_EQ(1, previous_count);
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldNotBlock(semaphore));
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Try to release more than the maximum.
  EXPECT_FALSE(semaphore->Release(101));
  EXPECT_TRUE(semaphore->Release(100));
  EXPECT_FALSE(semaphore->Release(1));
}

// Tests that semaphores wake threads.
TEST_F(SemaphoreTest, ReleaseWaking) {
  auto semaphore = Semaphore::Create(0, 2);
  EXPECT_NE(nullptr, semaphore);
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Spin up a thread that should block acquiring resources.
  std::atomic<bool> got_first{false};
  std::atomic<bool> got_second{false};
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Semaphore count = 0, expect blocking.
    EXPECT_TRUE(ShouldBlock(semaphore));

    // Continue test on the main thread.
    fence_event->Set();

    // Now actually wait until we are release. This will block.
    // The main thread should release twice.
    EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(semaphore));
    got_first = true;
    fence_event->Set();
    EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(semaphore));
    got_second = true;
    fence_event->Set();

    // Count is back to 0, should block.
    EXPECT_TRUE(ShouldBlock(semaphore));
  });

  // Wait until the thread hits the fence.
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));

  // Release first.
  EXPECT_FALSE(got_first);
  EXPECT_FALSE(got_second);
  EXPECT_TRUE(semaphore->Release());
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
  EXPECT_TRUE(got_first);
  EXPECT_FALSE(got_second);

  // Release second.
  EXPECT_TRUE(got_first);
  EXPECT_FALSE(got_second);
  EXPECT_TRUE(semaphore->Release());
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));
  EXPECT_TRUE(got_first);
  EXPECT_TRUE(got_second);

  // Wait for thread to cleanly exit.
  EXPECT_TRUE(thread->Join());

  // Semaphore should be back down to 0.
  EXPECT_TRUE(ShouldBlock(semaphore));
}

// Tests waiting on a signle semaphore and timeouts.
TEST_F(SemaphoreTest, WaitSemaphore) {
  auto semaphore = Semaphore::Create(0, 2);
  EXPECT_NE(nullptr, semaphore);

  // Waiting on a semaphore with a count == 0 should block.
  EXPECT_TRUE(ShouldBlock(semaphore));

  // Waiting on a semaphore with a count > 0 should pass immediately.
  EXPECT_TRUE(semaphore->Release(1));
  EXPECT_TRUE(ShouldNotBlock(semaphore));

  // Waiting on a semaphore with a count and a timeout should still pass
  // immediately.
  Stopwatch stopwatch;
  EXPECT_TRUE(semaphore->Release(1));
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::Wait(semaphore, std::chrono::milliseconds(100)));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waiting on an semaphore with count 0 and a timeout should wait a bit before
  // timing out.
  stopwatch.Reset();
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::Wait(semaphore, std::chrono::milliseconds(100)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waits should return before the timeout if the semaphore is released.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Wait a bit to ensure the main thread gets into its wait.
    Thread::Sleep(std::chrono::milliseconds(10));
    // Release the semaphore.
    EXPECT_TRUE(semaphore->Release());
  });
  fence_event->Set();
  stopwatch.Reset();
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::Wait(semaphore, std::chrono::seconds(100)));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(100));
}

// Tests signal and wait as a single operation.
TEST_F(SemaphoreTest, SignalAndWaitEvent) {
  auto semaphore_1 = Semaphore::Create(0, 2);
  auto semaphore_2 = Semaphore::Create(0, 2);

  // Release 1 and block trying to acquire 2.
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::SignalAndWait(semaphore_1, semaphore_2, kImmediateTimeout));
  EXPECT_TRUE(ShouldNotBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));

  // Release 1 and successfully acquire 2.
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
  EXPECT_TRUE(semaphore_2->Release());
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::SignalAndWait(semaphore_1, semaphore_2, kImmediateTimeout));
  EXPECT_TRUE(ShouldNotBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));

  // Verify timeout when waiting on a semaphore with count = 0.
  Stopwatch stopwatch;
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::SignalAndWait(semaphore_1, semaphore_2,
                                  std::chrono::milliseconds(100)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Attempting to release over the max should fail.
  semaphore_1 = Semaphore::Create(0, 1);
  semaphore_2 = Semaphore::Create(0, 1);
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
  semaphore_1->Release(1);               // count = 1, should now be at max.
  EXPECT_FALSE(semaphore_1->Release());  // At max.
  EXPECT_EQ(Thread::WaitResult::kError,
            Thread::SignalAndWait(semaphore_1, semaphore_2, kImmediateTimeout));
  EXPECT_TRUE(ShouldNotBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
}

// Tests waiting on any semaphore.
TEST_F(SemaphoreTest, WaitAnySemaphores) {
  auto semaphore_1 = Semaphore::Create(0, 2);
  auto semaphore_2 = Semaphore::Create(0, 2);

  // Waiting on a semaphore with a count == 0 should block.
  Thread::WaitAnyResult result =
      Thread::WaitAny({semaphore_1}, kImmediateTimeout);
  EXPECT_EQ(Thread::WaitResult::kTimeout, result.wait_result);

  // Waiting on a semaphore with a count > 0 should pass immediately.
  EXPECT_TRUE(semaphore_1->Release(1));
  result = Thread::WaitAny({semaphore_1}, kImmediateTimeout);
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(0, result.wait_handle_index);

  // Waiting on a semaphore with a count and a timeout should still pass
  // immediately.
  Stopwatch stopwatch;
  EXPECT_TRUE(semaphore_1->Release(1));
  result = Thread::WaitAny({semaphore_1}, std::chrono::milliseconds(100));
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(0, result.wait_handle_index);
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waiting on an semaphore with count 0 and a timeout should wait a bit before
  // timing out.
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  stopwatch.Reset();
  result = Thread::WaitAny({semaphore_1}, std::chrono::milliseconds(200));
  EXPECT_EQ(Thread::WaitResult::kTimeout, result.wait_result);
  EXPECT_EQ(0, result.wait_handle_index);
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waiting on a mix of semaphores should return the right index.
  EXPECT_TRUE(semaphore_1->Release());
  result = Thread::WaitAny({semaphore_1, semaphore_2});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(0, result.wait_handle_index);
  EXPECT_TRUE(semaphore_2->Release());
  result = Thread::WaitAny({semaphore_1, semaphore_2});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(1, result.wait_handle_index);

  // Waiting on multiple semaphores should only acquire one of them.
  EXPECT_TRUE(semaphore_1->Release());
  EXPECT_TRUE(semaphore_2->Release());
  result = Thread::WaitAny({semaphore_1, semaphore_2});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  if (result.wait_handle_index == 0) {
    EXPECT_TRUE(ShouldBlock(semaphore_1));
    EXPECT_TRUE(ShouldNotBlock(semaphore_2));
  } else if (result.wait_handle_index == 1) {
    EXPECT_TRUE(ShouldNotBlock(semaphore_1));
    EXPECT_TRUE(ShouldBlock(semaphore_2));
  }

  // Waits should return before the timeout if a semaphore is released.
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Wait a bit to ensure the main thread gets into its wait.
    Thread::Sleep(std::chrono::milliseconds(100));
    // Release a semaphore.
    EXPECT_TRUE(semaphore_2->Release());
  });
  fence_event->Set();
  result =
      Thread::WaitAny({semaphore_1, semaphore_2}, std::chrono::seconds(100));
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(1, result.wait_handle_index);
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
}

// Tests waiting on all semaphores.
TEST_F(SemaphoreTest, WaitAllSemaphores) {
  auto semaphore_1 = Semaphore::Create(0, 2);
  auto semaphore_2 = Semaphore::Create(0, 2);

  // Waiting on a semaphore with a count == 0 should block.
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({semaphore_1}, kImmediateTimeout));

  // Waiting on a semaphore with a count > 0 should pass immediately.
  EXPECT_TRUE(semaphore_1->Release(1));
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({semaphore_1}, kImmediateTimeout));

  // Waiting on a semaphore with a count and a timeout should still pass
  // immediately.
  Stopwatch stopwatch;
  EXPECT_TRUE(semaphore_1->Release(1));
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({semaphore_1}, std::chrono::milliseconds(100)));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waiting on an semaphore with count 0 and a timeout should wait a bit before
  // timing out.
  stopwatch.Reset();
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({semaphore_1}, std::chrono::milliseconds(200)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waiting on multiple semaphores should acquire all of them.
  EXPECT_TRUE(semaphore_1->Release());
  EXPECT_TRUE(semaphore_2->Release());
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({semaphore_1, semaphore_2}));
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));

  // Waits should return before the timeout if all semaphores are released.
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Wait a bit to ensure the main thread gets into its wait.
    Thread::Sleep(std::chrono::milliseconds(100));
    // Release the semaphores.
    EXPECT_TRUE(semaphore_1->Release());
    EXPECT_TRUE(semaphore_2->Release());
  });
  fence_event->Set();
  EXPECT_EQ(
      Thread::WaitResult::kSuccess,
      Thread::WaitAll({semaphore_1, semaphore_2}, std::chrono::seconds(100)));
  EXPECT_TRUE(ShouldBlock(semaphore_1));
  EXPECT_TRUE(ShouldBlock(semaphore_2));
}

}  // namespace
}  // namespace xrtl
