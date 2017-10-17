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

#include "xrtl/gfx/testing/graphics_test.h"

#include "absl/strings/str_join.h"
#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace testing {

namespace {

using xrtl::testing::diffing::DiffProvider;
using xrtl::testing::diffing::DiffResult;

}  // namespace

GraphicsTest::TestCaseState* GraphicsTest::test_case_state_ = nullptr;

void GraphicsTest::SetUpTestCase(absl::string_view golden_base_path,
                                 ref_ptr<ContextFactory> context_factory) {
  WTF_SCOPE0("GraphicsTest#SetUpTestCase");

  // Stash static configuration for all tests.
  auto test_case_state = new TestCaseState();

  // Setup a diff provider (chosen based on bazel select).
  test_case_state->diff_provider = DiffProvider::Create();
  CHECK(test_case_state->diff_provider) << "Unable to create diff provider";
  bool diff_provider_ok =
      test_case_state->diff_provider->Initialize(golden_base_path);
  CHECK(diff_provider_ok) << "Unable to initialize diff provider";

  // Validate context factory.
  CHECK(context_factory) << "No context factory specified";
  CHECK(context_factory->default_device()) << "No graphics devices available";
  test_case_state->context_factory = std::move(context_factory);

  test_case_state_ = test_case_state;
}

void GraphicsTest::TearDownTestCase() {
  WTF_SCOPE0("GraphicsTest#TearDownTestCase");
  delete test_case_state_;
}

void GraphicsTest::SetUp() {
  WTF_SCOPE0("GraphicsTest#SetUp");

  auto test_state = absl::make_unique<TestState>();

  // Set required features/extensions.
  // TODO(benvanik): something sensible.
  gfx::Device::Features required_features;

  // Attempt to create the context.
  auto context_factory = test_case_state_->context_factory;
  auto create_result =
      context_factory->CreateContext(context_factory->default_device(),
                                     required_features, &test_state->context);
  CHECK_EQ(ContextFactory::CreateResult::kSuccess, create_result)
      << "Failed to create graphics context";

  // Allocate a memory heap to allocate buffers and textures.
  test_state->memory_heap = test_state->context->CreateMemoryHeap(
      MemoryType::kHostVisible | MemoryType::kHostCoherent, 16 * 1024 * 1024);
  CHECK(test_state->memory_heap) << "Failed to allocate memory heap";

  test_state_ = std::move(test_state);
}

void GraphicsTest::TearDown() {
  WTF_SCOPE0("GraphicsTest#TearDown");
  auto test_state = std::move(test_state_);
  test_state_.reset();
  test_state->context->WaitUntilQueuesIdle();
  test_state->memory_heap.reset();
  test_state->context.reset();
  test_state.reset();
}

bool GraphicsTest::SubmitAndWait(ref_ptr<CommandBuffer> command_buffer) {
  WTF_SCOPE0("GraphicsTest#SubmitAndWait");

  // Submit the command buffer for execution.
  // We'll just wait for full idle so no need for synchronization primitives.
  Context::SubmitResult submit_result =
      test_context()->Submit({command_buffer}, {});
  switch (submit_result) {
    case Context::SubmitResult::kSuccess:
      break;
    case Context::SubmitResult::kCommandBufferReused:
    case Context::SubmitResult::kDeviceLost:
      LOG(ERROR) << "Command buffer submission failed: " << submit_result;
      return false;
  }

  // Wait until all queues are idle. We could be more specific with the
  // synchronization primitives provided with Context::Submit, but this is good
  // enough for testing.
  Context::WaitResult wait_result = test_context()->WaitUntilQueuesIdle();
  switch (wait_result) {
    case Context::WaitResult::kSuccess:
      break;
    case Context::WaitResult::kDeviceLost:
      LOG(ERROR) << "Wait for idle after submission failed: " << wait_result;
      return false;
  }

  return true;
}

bool GraphicsTest::CompareBuffer(
    const std::vector<uint8_t>& expected_data, ref_ptr<Buffer> buffer,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::DataDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#CompareBuffer");
  CHECK(false) << "Not yet implemented";
  return false;
}

bool GraphicsTest::SubmitAndCompareBuffer(
    const std::vector<uint8_t>& expected_data,
    ref_ptr<CommandBuffer> command_buffer, ref_ptr<Buffer> buffer,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::DataDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#SubmitAndCompareBuffer");
  if (!SubmitAndWait(std::move(command_buffer))) {
    return false;
  }
  return CompareBuffer(expected_data, std::move(buffer), publish_mode,
                       std::move(options));
}

bool GraphicsTest::CompareBuffer(
    absl::string_view test_key, ref_ptr<Buffer> buffer,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::DataDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#CompareBuffer");
  CHECK(false) << "Not yet implemented";
  return false;
}

bool GraphicsTest::SubmitAndCompareBuffer(
    absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
    ref_ptr<Buffer> buffer,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::DataDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#SubmitAndCompareBuffer");
  if (!SubmitAndWait(std::move(command_buffer))) {
    return false;
  }
  return CompareBuffer(test_key, std::move(buffer), publish_mode,
                       std::move(options));
}

bool GraphicsTest::CompareImage(
    absl::string_view test_key, ref_ptr<ImageView> image_view,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::ImageDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#CompareImage");

  // TODO(benvanik): support for non-k2D Image::Types.
  CHECK(image_view->type() == Image::Type::k2D)
      << "Only k2D images are supported right now";
  // TODO(benvanik): support for other formats/conversion.
  CHECK(image_view->format() == PixelFormats::kR8G8B8A8UNorm)
      << "Only kR8G8B8A8UNorm images are supported right now";

  // Prepare our heap buffer for population.
  int data_width = image_view->size().width;
  int data_height = image_view->size().height;
  int channels = 4;
  auto image_buffer =
      xrtl::testing::ImageBuffer::Create(data_width, data_height, channels);

  // Read back the image contents into a byte buffer.
  if (!test_context()->ReadImageData(
          image_view->image(),
          {{image_view->layer_range(), image_buffer->data(),
            image_buffer->data_size()}})) {
    LOG(ERROR) << "Failed to read back image contents";
    return false;
  }

  // Defer comparison to the diff provider.
  DiffResult diff_result = test_case_state_->diff_provider->CompareImage(
      test_key, image_buffer.get(), publish_mode, std::move(options));
  return diff_result == DiffResult::kEquivalent;
}

bool GraphicsTest::SubmitAndCompareImage(
    absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
    ref_ptr<ImageView> image_view,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::ImageDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#SubmitAndCompareImage");
  if (!SubmitAndWait(std::move(command_buffer))) {
    return false;
  }
  return CompareImage(test_key, std::move(image_view), publish_mode,
                      std::move(options));
}

bool GraphicsTest::CompareFramebuffer(
    absl::string_view test_key, ref_ptr<Framebuffer> framebuffer,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::ImageDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#CompareFramebuffer");
  bool all_passed = true;
  for (size_t i = 0; i < framebuffer->attachments().size(); ++i) {
    ref_ptr<ImageView> image_view = framebuffer->attachments()[i];
    if (!CompareImage(
            absl::StrJoin({std::string(test_key), std::to_string(i)}, "_"),
            image_view, publish_mode, options)) {
      all_passed = false;
    }
  }
  return all_passed;
}

bool GraphicsTest::SubmitAndCompareFramebuffer(
    absl::string_view test_key, ref_ptr<CommandBuffer> command_buffer,
    ref_ptr<Framebuffer> framebuffer,
    xrtl::testing::diffing::DiffPublishMode publish_mode,
    xrtl::testing::diffing::ImageDiffer::Options options) {
  WTF_SCOPE0("GraphicsTest#SubmitAndCompareFramebuffer");
  if (!SubmitAndWait(std::move(command_buffer))) {
    return false;
  }
  return CompareFramebuffer(test_key, std::move(framebuffer), publish_mode,
                            std::move(options));
}

}  // namespace testing
}  // namespace gfx
}  // namespace xrtl
