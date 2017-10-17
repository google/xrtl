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

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace testing {
namespace {

using gfx::RenderPass;

class GraphicsTestTest : public GraphicsTest {
 public:
  static void SetUpTestCase() {
    GraphicsTest::SetUpTestCase("xrtl/gfx/testing/testdata/goldens",
                                ContextFactory::Create());
  }

  struct PatternBuffer {
    std::vector<uint8_t> expected_data;
    ref_ptr<Buffer> buffer;
  };

  // Creates a buffer with somewhat predictable contents.
  absl::optional<PatternBuffer> CreatePatternBuffer(size_t buffer_size) {
    // Scribble memory.
    // TODO(benvanik): something clever (static random data?).
    std::vector<uint8_t> expected_data;
    expected_data.resize(buffer_size);
    for (size_t i = 0; i < expected_data.size(); ++i) {
      expected_data[i] = static_cast<uint8_t>(i % 256);
    }

    // Allocate buffer memory.
    ref_ptr<Buffer> buffer;
    auto allocation_result = test_memory_heap()->AllocateBuffer(
        expected_data.size(),
        Buffer::Usage::kTransferSource | Buffer::Usage::kTransferTarget,
        &buffer);
    switch (allocation_result) {
      case MemoryHeap::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate buffer";
        return absl::nullopt;
    }

    // Write data directly into the buffer.
    // A real app would want to use a staging buffer.
    if (!test_context()->WriteBufferData(
            buffer, {{0, expected_data.data(), expected_data.size()}})) {
      LOG(ERROR) << "Failed to write data into buffer";
      return absl::nullopt;
    }

    PatternBuffer pattern_buffer;
    pattern_buffer.expected_data = std::move(expected_data);
    pattern_buffer.buffer = std::move(buffer);
    return pattern_buffer;
  }

  // Creates a width x height 2-tone RGBA grid image.
  ref_ptr<Image> CreateGridImage(int width, int height) {
    // Draw a grid.
    auto grid_image_buffer =
        xrtl::testing::ImageBuffer::Create(width, height, 4);
    grid_image_buffer->DrawGrid(8, 0xFF0000FF, 0x0000FFFF);

    // Allocate image memory.
    gfx::Image::CreateParams create_params;
    create_params.format = gfx::PixelFormats::kB8G8R8A8UNorm;
    create_params.tiling_mode = Image::TilingMode::kLinear;
    create_params.size = {grid_image_buffer->data_width(),
                          grid_image_buffer->data_height()};
    create_params.initial_layout = Image::Layout::kPreinitialized;
    ref_ptr<Image> grid_image;
    auto allocation_result = test_memory_heap()->AllocateImage(
        create_params, Image::Usage::kTransferTarget | Image::Usage::kSampled,
        &grid_image);
    switch (allocation_result) {
      case MemoryHeap::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate texture image";
        return nullptr;
    }

    // Write data directly into the image.
    // A real app would want to use a staging buffer.
    if (!test_context()->WriteImageData(
            grid_image, {{grid_image->entire_range(), grid_image_buffer->data(),
                          grid_image_buffer->data_size()}})) {
      LOG(ERROR) << "Failed to write data into texture image";
      return nullptr;
    }

    return grid_image;
  }
};

// Tests simple readback and comparison of a Buffer against an inlined value.
TEST_F(GraphicsTestTest, CompareBufferInline) {
  auto pattern_buffer_opt = CreatePatternBuffer(64 * 1024);
  ASSERT_TRUE(pattern_buffer_opt);
  auto& pattern_buffer = pattern_buffer_opt.value();

  // Test the full buffer contents.
  EXPECT_TRUE(CompareBuffer(pattern_buffer.expected_data, pattern_buffer.buffer,
                            0, pattern_buffer.expected_data.size()));

  // Test a small subregion of the buffer.
  std::vector<uint8_t> partial_expected_data{
      pattern_buffer.expected_data.begin() + 512,
      pattern_buffer.expected_data.begin() + 512 + 600};
  EXPECT_TRUE(
      CompareBuffer(partial_expected_data, pattern_buffer.buffer, 512, 600));
}

// Tests simple readback and comparison of a Buffer against a golden.
TEST_F(GraphicsTestTest, CompareBufferGolden) {
  auto pattern_buffer_opt = CreatePatternBuffer(64 * 1024);
  ASSERT_TRUE(pattern_buffer_opt);
  auto& pattern_buffer = pattern_buffer_opt.value();

  // Test the full buffer contents.
  EXPECT_TRUE(CompareBuffer("compare_buffer_golden_full", pattern_buffer.buffer,
                            0, pattern_buffer.expected_data.size()));

  // Test a small subregion of the buffer.
  EXPECT_TRUE(CompareBuffer("compare_buffer_golden_partial",
                            pattern_buffer.buffer, 512, 600));
}

// Tests simple readback and comparison of an Image.
TEST_F(GraphicsTestTest, CompareImage) {
  auto grid_image = CreateGridImage(128, 128);
  EXPECT_NE(nullptr, grid_image);
  EXPECT_TRUE(CompareImage("compare_image", std::move(grid_image)));
}

// Tests simple readback and comparison of a Framebuffer.
TEST_F(GraphicsTestTest, CompareFramebuffer) {
  // Configure render pass for clearing the framebuffer.
  RenderPass::AttachmentDescription color_attachment;
  color_attachment.format = gfx::PixelFormats::kB8G8R8A8UNorm;
  color_attachment.load_op = RenderPass::LoadOp::kClear;
  color_attachment.store_op = RenderPass::StoreOp::kStore;
  color_attachment.initial_layout = Image::Layout::kUndefined;
  color_attachment.final_layout = Image::Layout::kGeneral;
  RenderPass::SubpassDescription subpass;
  subpass.color_attachments.push_back(
      {0, Image::Layout::kColorAttachmentOptimal});
  subpass.color_attachments.push_back(
      {1, Image::Layout::kColorAttachmentOptimal});
  auto render_pass = test_context()->CreateRenderPass(
      {color_attachment, color_attachment}, {subpass}, {});
  ASSERT_NE(nullptr, render_pass);

  // Create backing images for the framebuffer.
  gfx::Image::CreateParams image_create_params;
  image_create_params.format = gfx::PixelFormats::kB8G8R8A8UNorm;
  image_create_params.tiling_mode = Image::TilingMode::kLinear;
  image_create_params.size = {128, 128};
  image_create_params.initial_layout = Image::Layout::kUndefined;
  ref_ptr<Image> image_0;
  test_memory_heap()->AllocateImage(
      image_create_params,
      Image::Usage::kTransferSource | Image::Usage::kColorAttachment, &image_0);
  ASSERT_NE(nullptr, image_0);
  ref_ptr<Image> image_1;
  test_memory_heap()->AllocateImage(
      image_create_params,
      Image::Usage::kTransferSource | Image::Usage::kColorAttachment, &image_1);
  ASSERT_NE(nullptr, image_1);
  auto image_view_0 = image_0->CreateView();
  auto image_view_1 = image_1->CreateView();

  // Allocate framebuffer.
  auto framebuffer = test_context()->CreateFramebuffer(
      render_pass, {128, 128}, {image_view_0, image_view_1});
  ASSERT_NE(nullptr, framebuffer);

  // Record render pass that just clears the framebuffer attachments.
  auto command_buffer = test_context()->CreateCommandBuffer();
  ASSERT_NE(nullptr, command_buffer);
  auto rpe = command_buffer->BeginRenderPass(
      render_pass, framebuffer,
      {gfx::ClearColor(1.0f, 0.0f, 0.0f, 1.0f),
       gfx::ClearColor(0.0f, 0.0f, 1.0f, 1.0f)});
  command_buffer->EndRenderPass(std::move(rpe));

  // Issue and compare.
  EXPECT_TRUE(SubmitAndCompareFramebuffer("compare_framebuffer",
                                          std::move(command_buffer),
                                          std::move(framebuffer)));
}

}  // namespace
}  // namespace testing
}  // namespace gfx
}  // namespace xrtl
