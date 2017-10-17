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

#ifndef XRTL_GFX_TESTING_GRAPHICS_TEST_H_
#define XRTL_GFX_TESTING_GRAPHICS_TEST_H_

#include <memory>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "xrtl/gfx/context.h"
#include "xrtl/gfx/context_factory.h"
#include "xrtl/testing/diffing/diff_provider.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace testing {

// Base tests for graphics code that requires a graphics context.
// This will ensure each test case runs with a fresh context and provides
// utilities for comparing gfx::Buffer and gfx::Image results with goldens.
//
// Usage:
//  class MyTest : public gfx::testing::GraphicsTest {
//   public:
//    static void SetUpTestCase() {
//      GraphicsTest::SetUpTestCase("package/.../testdata/goldens");
//    }
//  };
//  TEST_F(MyTest, DoSomething) {
//    auto command_buffer = test_context()->CreateCommandBuffer();
//    auto framebuffer = test_context()->CreateFramebuffer(...);
//    auto rpe = command_buffer->BeginRenderPass(..., framebuffer, ...);
//    rpe->Draw(...);
//    command_buffer->EndRenderPass(...);
//    EXPECT_TRUE(SubmitAndCompareFramebuffer("do_something", command_buffer,
//                                            frame_buffer));
//  }
class GraphicsTest : public ::testing::Test {
 protected:
  static void SetUpTestCase(absl::string_view golden_base_path,
                            ref_ptr<ContextFactory> context_factory);
  static void TearDownTestCase();

  void SetUp() override;
  void TearDown() override;

  // A context created for each test case.
  ref_ptr<Context> test_context() {
    CHECK(test_state_) << "Only available within a testcase";
    return test_state_->context;
  }

  // A memory heap bound to the test_context that can be used for allocations.
  ref_ptr<MemoryHeap> test_memory_heap() {
    CHECK(test_state_) << "Only available within a testcase";
    return test_state_->memory_heap;
  }

  // Submits a command buffer for execution and waits until all queues are idle.
  // Returns false if the command buffer failed to submit or an error occurred
  // while waiting for idle.
  bool SubmitAndWait(ref_ptr<CommandBuffer> command_buffer);

  // Compares the data in the buffer to the given expected data.
  // Returns true if the expected data and the resulting buffer match exactly.
  bool CompareBuffer(const std::vector<uint8_t>& expected_data,
                     ref_ptr<Buffer> buffer, size_t buffer_offset,
                     size_t buffer_length,
                     xrtl::testing::diffing::DiffPublishMode publish_mode =
                         xrtl::testing::diffing::DiffPublishMode::kFailure,
                     xrtl::testing::diffing::DataDiffer::Options options = {});

  // Submits a command buffer and compares the data in the resulting buffer to
  // the given expected data.
  // Returns true if the expected data and the resulting buffer match exactly.
  bool SubmitAndCompareBuffer(
      const std::vector<uint8_t>& expected_data,
      ref_ptr<CommandBuffer> command_buffer, ref_ptr<Buffer> buffer,
      size_t buffer_offset, size_t buffer_length,
      xrtl::testing::diffing::DiffPublishMode publish_mode =
          xrtl::testing::diffing::DiffPublishMode::kFailure,
      xrtl::testing::diffing::DataDiffer::Options options = {});

  // Compares the data in the buffer to the golden specified by test_key.
  // Returns true if the golden data and the resulting buffer match exactly.
  bool CompareBuffer(absl::string_view test_key, ref_ptr<Buffer> buffer,
                     size_t buffer_offset, size_t buffer_length,
                     xrtl::testing::diffing::DiffPublishMode publish_mode =
                         xrtl::testing::diffing::DiffPublishMode::kFailure,
                     xrtl::testing::diffing::DataDiffer::Options options = {});

  // Submits a command buffer and compares the data in the resulting buffer to
  // the golden specified by test_key.
  // Returns true if the golden data and the resulting buffer match exactly.
  bool SubmitAndCompareBuffer(
      absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
      ref_ptr<Buffer> buffer, size_t buffer_offset, size_t buffer_length,
      xrtl::testing::diffing::DiffPublishMode publish_mode =
          xrtl::testing::diffing::DiffPublishMode::kFailure,
      xrtl::testing::diffing::DataDiffer::Options options = {});

  // Compares the image to the golden specified by test_key.
  // Returns true if the expected data and the resulting image match exactly.
  bool CompareImage(absl::string_view test_key, ref_ptr<ImageView> image_view,
                    xrtl::testing::diffing::DiffPublishMode publish_mode =
                        xrtl::testing::diffing::DiffPublishMode::kFailure,
                    xrtl::testing::diffing::ImageDiffer::Options options = {});
  bool CompareImage(absl::string_view test_key, ref_ptr<Image> image,
                    xrtl::testing::diffing::DiffPublishMode publish_mode =
                        xrtl::testing::diffing::DiffPublishMode::kFailure,
                    xrtl::testing::diffing::ImageDiffer::Options options = {}) {
    return CompareImage(test_key, image->CreateView(), publish_mode,
                        std::move(options));
  }

  // Submits a command buffer and compares the resulting image to the golden
  // specified by test_key.
  // Returns true if the expected data and the resulting image match exactly.
  bool SubmitAndCompareImage(
      absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
      ref_ptr<ImageView> image_view,
      xrtl::testing::diffing::DiffPublishMode publish_mode =
          xrtl::testing::diffing::DiffPublishMode::kFailure,
      xrtl::testing::diffing::ImageDiffer::Options options = {});
  bool SubmitAndCompareImage(
      absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
      ref_ptr<Image> image,
      xrtl::testing::diffing::DiffPublishMode publish_mode =
          xrtl::testing::diffing::DiffPublishMode::kFailure,
      xrtl::testing::diffing::ImageDiffer::Options options = {}) {
    return SubmitAndCompareImage(test_key, std::move(command_buffer),
                                 image->CreateView(), publish_mode,
                                 std::move(options));
  }

  // Compares the data in the framebuffer to the golden specified by test_key.
  // Returns true if the golden data and the resulting buffer match exactly.
  bool CompareFramebuffer(
      absl::string_view test_key, ref_ptr<Framebuffer> framebuffer,
      xrtl::testing::diffing::DiffPublishMode publish_mode =
          xrtl::testing::diffing::DiffPublishMode::kFailure,
      xrtl::testing::diffing::ImageDiffer::Options options = {});

  // Submits a command buffer and compares the data in the resulting framebuffer
  // to the golden specified by test_key.
  // Returns true if the golden data and the resulting buffer match exactly.
  bool SubmitAndCompareFramebuffer(
      absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
      ref_ptr<Framebuffer> framebuffer,
      xrtl::testing::diffing::DiffPublishMode publish_mode =
          xrtl::testing::diffing::DiffPublishMode::kFailure,
      xrtl::testing::diffing::ImageDiffer::Options options = {});

 private:
  // Shared for all tests within the suite.
  struct TestCaseState {
    std::unique_ptr<xrtl::testing::diffing::DiffProvider> diff_provider;
    ref_ptr<ContextFactory> context_factory;
  };
  static TestCaseState* test_case_state_;

  // Recreated for each test.
  struct TestState {
    ref_ptr<Context> context;
    ref_ptr<MemoryHeap> memory_heap;
  };
  std::unique_ptr<TestState> test_state_;
};

}  // namespace testing
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_TESTING_GRAPHICS_TEST_H_
