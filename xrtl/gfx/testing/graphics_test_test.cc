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

class GraphicsTestTest : public GraphicsTest {
 public:
  static void SetUpTestCase() {
    GraphicsTest::SetUpTestCase("xrtl/gfx/testing/testdata/goldens",
                                ContextFactory::Create());
  }

  // Creates a width x height 2-tone RGBA grid image.
  ref_ptr<Image> CreateGridImage(int width, int height) {
    auto grid_image_buffer =
        xrtl::testing::ImageBuffer::Create(width, height, 4);
    grid_image_buffer->DrawGrid(8, 0xFF0000FF, 0x0000FFFF);

    gfx::Image::CreateParams create_params;
    create_params.format = gfx::PixelFormats::kR8G8B8A8UNorm;
    create_params.tiling_mode = Image::TilingMode::kLinear;
    create_params.size = {grid_image_buffer->data_width(),
                          grid_image_buffer->data_height()};
    create_params.initial_layout = Image::Layout::kPreinitialized;

    ref_ptr<Image> grid_image;
    auto allocation_result = test_memory_heap()->AllocateImage(
        create_params, Image::Usage::kSampled, &grid_image);
    switch (allocation_result) {
      case MemoryHeap::AllocationResult::kSuccess:
        break;
      default:
        LOG(ERROR) << "Failed to allocate texture image";
        return nullptr;
    }

    // Write data directly into the image.
    // A real app would want to use a staging buffer.
    if (!grid_image->WriteData(grid_image->entire_range(),
                               grid_image_buffer->data(),
                               grid_image_buffer->data_size())) {
      LOG(ERROR) << "Failed to write data into texture image";
      return nullptr;
    }

    return grid_image;
  }
};

// Tests simple readback and comparison of an Image.
TEST_F(GraphicsTestTest, CompareImage) {
  auto grid_image = CreateGridImage(128, 128);
  EXPECT_TRUE(CompareImage("compare_image", std::move(grid_image)));
}

}  // namespace
}  // namespace testing
}  // namespace gfx
}  // namespace xrtl
