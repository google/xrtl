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

#include "xrtl/base/stopwatch.h"
#include "xrtl/base/threading/thread.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace {

bool ShouldBlock(ref_ptr<Event> event) {
  return Thread::Wait(event, kImmediateTimeout) == Thread::WaitResult::kTimeout;
}

bool ShouldNotBlock(ref_ptr<Event> event) {
  return Thread::Wait(event, kImmediateTimeout) == Thread::WaitResult::kSuccess;
}

class EventTest : public ::testing::Test {
 public:
  // We need high resolution timing to ensure our waits are measured correctly.
  static void SetUpTestCase() { Process::EnableHighResolutionTiming(); }
  static void TearDownTestCase() { Process::DisableHighResolutionTiming(); }
};

// Tests the basic behavior of a manual reset event.
TEST_F(EventTest, ManualResetEvent) {
  // Create unset and expect blocking.
  auto event = Event::CreateManualResetEvent(false);
  EXPECT_NE(nullptr, event);
  EXPECT_TRUE(ShouldBlock(event));

  // Set and expect passing.
  event->Set();
  EXPECT_TRUE(ShouldNotBlock(event));

  // Reset and expect blocking again.
  event->Reset();
  EXPECT_TRUE(ShouldBlock(event));
}

// Tests a manual reset event with an initial value of signaled.
TEST_F(EventTest, ManualResetEventInitiallySet) {
  // Create set and expect passing.
  auto event = Event::CreateManualResetEvent(true);
  EXPECT_NE(nullptr, event);
  EXPECT_TRUE(ShouldNotBlock(event));

  // Reset and expect blocking.
  event->Reset();
  EXPECT_TRUE(ShouldBlock(event));
}

// Tests that manual reset events wake waiting threads.
TEST_F(EventTest, ManualResetEventWaking) {
  // Create unset and expect blocking.
  auto event = Event::CreateManualResetEvent(false);
  EXPECT_NE(nullptr, event);
  EXPECT_TRUE(ShouldBlock(event));

  // Spin up a thread that should block on the event.
  auto fence_event = Event::CreateManualResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Expect us to block.
    EXPECT_TRUE(ShouldBlock(event));

    // Continue test on the main thread.
    fence_event->Set();

    // Now actually wait until we are signaled. This will block.
    EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(event));
  });

  // Wait until the thread hits the fence.
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));

  // Set the event and let the thread return.
  event->Set();

  // Wait for thread to cleanly exit.
  EXPECT_TRUE(thread->Join());
}

// Tests the basic behavior of an auto reset event.
TEST_F(EventTest, AutoResetEvent) {
  // Create unset and expect blocking.
  auto event = Event::CreateAutoResetEvent(false);
  EXPECT_NE(nullptr, event);
  EXPECT_TRUE(ShouldBlock(event));

  // Set and expect passing.
  event->Set();
  EXPECT_TRUE(ShouldNotBlock(event));

  // The event should have been automatically reset and block.
  EXPECT_TRUE(ShouldBlock(event));

  // Set and then manually reset. Should block.
  event->Set();
  event->Reset();
  EXPECT_TRUE(ShouldBlock(event));
}

// Tests an auto reset event with an initial value of signaled.
TEST_F(EventTest, AutoResetEventInitiallySet) {
  // Create set and expect passing.
  auto event = Event::CreateAutoResetEvent(true);
  EXPECT_NE(nullptr, event);
  EXPECT_TRUE(ShouldNotBlock(event));

  // The event should have been automatically reset and block.
  EXPECT_TRUE(ShouldBlock(event));
}

// Tests that auto reset events wake waiting threads.
TEST_F(EventTest, AutoResetEventWaking) {
  // Create unset and expect blocking.
  auto event = Event::CreateAutoResetEvent(false);
  EXPECT_TRUE(ShouldBlock(event));

  // Spin up a thread that should block on the event.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Expect us to block.
    EXPECT_TRUE(ShouldBlock(event));

    // Continue test on the main thread.
    fence_event->Set();

    // Now actually wait until we are signaled. This will block.
    EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(event));

    // Event will be auto reset to unsignaled and should block.
    EXPECT_TRUE(ShouldBlock(event));
  });

  // Wait until the thread hits the fence.
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::Wait(fence_event));

  // Set the event and let the thread return.
  event->Set();

  // Wait for thread to cleanly exit.
  EXPECT_TRUE(thread->Join());

  // Event should have been reset to unsignaled.
  EXPECT_TRUE(ShouldBlock(event));
}

// Tests waiting on a single event and timeouts.
TEST_F(EventTest, WaitEvent) {
  auto signaled_event = Event::CreateManualResetEvent(true);
  auto unsignaled_event = Event::CreateManualResetEvent(false);

  // Waiting on a signaled event should pass immediately.
  EXPECT_TRUE(ShouldNotBlock(signaled_event));

  // Waiting on a signaled event with a timeout should still pass immediately.
  Stopwatch stopwatch;
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::Wait(signaled_event, std::chrono::milliseconds(100)));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waiting on an unsignaled event should block.
  EXPECT_TRUE(ShouldBlock(unsignaled_event));

  // Waiting on an unsignaled event with a timeout should wait a bit before
  // timing out.
  stopwatch.Reset();
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::Wait(unsignaled_event, std::chrono::milliseconds(100)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waits should return before the timeout if the event is signaled.
  auto early_event = Event::CreateManualResetEvent(false);
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Wait a bit to ensure the main thread gets into its wait.
    Thread::Sleep(std::chrono::milliseconds(10));
    // Signal the event.
    early_event->Set();
  });
  fence_event->Set();
  stopwatch.Reset();
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::Wait(early_event, std::chrono::seconds(100)));
  EXPECT_LE(stopwatch.elapsed_micros(), std::chrono::milliseconds(100));
}

// Tests signal and wait as a single operation.
TEST_F(EventTest, SignalAndWaitEvent) {
  auto signaled_event = Event::CreateManualResetEvent(true);
  auto unsignaled_event = Event::CreateManualResetEvent(false);

  // No-op: signal the signaled event and wait on the unsignaled event.
  EXPECT_TRUE(ShouldNotBlock(signaled_event));
  EXPECT_TRUE(ShouldBlock(unsignaled_event));
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::SignalAndWait(signaled_event, unsignaled_event,
                                  kImmediateTimeout));
  EXPECT_TRUE(ShouldNotBlock(signaled_event));
  EXPECT_TRUE(ShouldBlock(unsignaled_event));

  // Signal the unsignaled event and wait on the signaled event.
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::SignalAndWait(unsignaled_event, signaled_event,
                                  kImmediateTimeout));
  // Unsignaled should be signaled by the call and should not block.
  EXPECT_TRUE(ShouldNotBlock(unsignaled_event));
  EXPECT_TRUE(ShouldNotBlock(signaled_event));
  unsignaled_event->Reset();
  signaled_event->Set();

  // Waiting on an unsignaled event with a timeout should wait a bit before
  // timing out.
  Stopwatch stopwatch;
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::SignalAndWait(signaled_event, unsignaled_event,
                                  std::chrono::milliseconds(100)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));
}

// Tests waiting on any event.
TEST_F(EventTest, WaitAnyEvents) {
  auto signaled_event_1 = Event::CreateManualResetEvent(true);
  auto signaled_event_2 = Event::CreateManualResetEvent(true);
  auto unsignaled_event_1 = Event::CreateManualResetEvent(false);
  auto unsignaled_event_2 = Event::CreateManualResetEvent(false);

  // Waiting on a signaled event should pass immediately.
  Thread::WaitAnyResult result = Thread::WaitAny({signaled_event_1});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(0, result.wait_handle_index);
  // Waiting on an unsignaled event should block.
  result = Thread::WaitAny({unsignaled_event_1}, kImmediateTimeout);
  EXPECT_EQ(Thread::WaitResult::kTimeout, result.wait_result);

  // Waiting on a mix of events should return the right index.
  result = Thread::WaitAny({signaled_event_1, unsignaled_event_1});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(0, result.wait_handle_index);
  result = Thread::WaitAny({unsignaled_event_1, signaled_event_1});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(1, result.wait_handle_index);

  // Waiting on multiple signaled events should succeed.
  result = Thread::WaitAny({signaled_event_1, signaled_event_2});
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_LE(0, result.wait_handle_index);
  EXPECT_GE(1, result.wait_handle_index);

  // Waiting on an unsignaled event with a timeout should wait a bit before
  // timing out.
  Stopwatch stopwatch;
  result = Thread::WaitAny({unsignaled_event_1, unsignaled_event_2},
                           std::chrono::milliseconds(100));
  EXPECT_EQ(Thread::WaitResult::kTimeout, result.wait_result);
  EXPECT_GT(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waits should return before the timeout if an event is signaled.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Wait a bit to ensure the main thread gets into its wait.
    Thread::Sleep(std::chrono::milliseconds(100));
    // Signal the event.
    unsignaled_event_2->Set();
  });
  fence_event->Set();
  result = Thread::WaitAny({unsignaled_event_1, unsignaled_event_2},
                           std::chrono::seconds(100));
  EXPECT_EQ(Thread::WaitResult::kSuccess, result.wait_result);
  EXPECT_EQ(1, result.wait_handle_index);
  unsignaled_event_2->Reset();
}

// Tests waiting on all events.
TEST_F(EventTest, WaitAllEvents) {
  auto signaled_event_1 = Event::CreateManualResetEvent(true);
  auto signaled_event_2 = Event::CreateManualResetEvent(true);
  auto unsignaled_event_1 = Event::CreateManualResetEvent(false);
  auto unsignaled_event_2 = Event::CreateManualResetEvent(false);

  // Waiting on a signaled event should pass immediately.
  EXPECT_EQ(Thread::WaitResult::kSuccess, Thread::WaitAll({signaled_event_1}));
  // Waiting on an unsignaled event should block.
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({unsignaled_event_1}, kImmediateTimeout));

  // Waiting on a mix of events should return only when all are set.
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({signaled_event_1, signaled_event_2}));

  // Waiting on an unsignaled event with a timeout should wait a bit before
  // timing out.
  Stopwatch stopwatch;
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({signaled_event_1, unsignaled_event_1},
                            std::chrono::milliseconds(100)));
  EXPECT_GE(stopwatch.elapsed_micros(), std::chrono::milliseconds(10));

  // Waits should timeout if not all events are set.
  auto fence_event = Event::CreateAutoResetEvent(false);
  auto thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Signal one of the events after a moment.
    Thread::Sleep(std::chrono::milliseconds(100));
    unsignaled_event_2->Set();
  });
  fence_event->Set();
  EXPECT_EQ(Thread::WaitResult::kTimeout,
            Thread::WaitAll({unsignaled_event_1, unsignaled_event_2},
                            std::chrono::milliseconds(100)));

  // Waits should return before the timeout if all events are signaled.
  thread = Thread::Create({}, [&]() {
    // Wait until the main thread enters its wait.
    Thread::Wait(fence_event);
    // Wait a bit to ensure the main thread gets into its wait.
    Thread::Sleep(std::chrono::milliseconds(100));
    // Signal the events.
    unsignaled_event_1->Set();
    unsignaled_event_2->Set();
  });
  fence_event->Set();
  EXPECT_EQ(Thread::WaitResult::kSuccess,
            Thread::WaitAll({unsignaled_event_1, unsignaled_event_2},
                            std::chrono::seconds(100)));
  unsignaled_event_1->Reset();
  unsignaled_event_2->Reset();
}

}  // namespace
}  // namespace xrtl
