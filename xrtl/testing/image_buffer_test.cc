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

#include "xrtl/testing/image_buffer.h"

#include "xrtl/testing/diffing/image_differ.h"
#include "xrtl/testing/file_util.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace testing {
namespace {

// Creates a grid equivalent to the testdata rgb.png file with the given number
// of channels.
std::unique_ptr<ImageBuffer> CreateGridRGB(int channels) {
  // Create an RGB image with display/data dimensions differing and fill so that
  // we can validate only the display area is saved.
  auto image_buffer = ImageBuffer::Create(16, 8, channels);
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  image_buffer->set_display_size(8, 4);
  const uint8_t kColorA[4] = {0xFF, 0x22, 0x00, 0xDD};
  const uint8_t kColorB[4] = {0x00, 0x55, 0xFF, 0x44};
  image_buffer->DrawGrid(2, kColorA, kColorB);
  image_buffer->set_display_size(image_buffer->data_width(),
                                 image_buffer->data_height());
  return image_buffer;
}

// Compares the given compressed data buffers and ensures they are bit-for-bit
// identical.
bool CompareCompressedData(const std::vector<uint8_t>& compressed_data_expected,
                           const std::vector<uint8_t>& compressed_data_actual) {
  EXPECT_EQ(compressed_data_expected.size(), compressed_data_actual.size());
  if (compressed_data_expected.size() != compressed_data_actual.size()) {
    return false;  // Avoid memcmp if sizes differ.
  }
  bool is_data_equal = std::memcmp(compressed_data_expected.data(),
                                   compressed_data_actual.data(),
                                   compressed_data_expected.size()) == 0;
  EXPECT_TRUE(is_data_equal) << "One or more bytes differ";
  return is_data_equal;
}

// Creates a new empty image.
TEST(ImageBufferTest, Empty) {
  auto image_buffer = ImageBuffer::Create(200, 150, 3);
  EXPECT_EQ(200, image_buffer->data_width());
  EXPECT_EQ(150, image_buffer->data_height());
  EXPECT_EQ(200, image_buffer->display_width());
  EXPECT_EQ(150, image_buffer->display_height());
  EXPECT_EQ(3, image_buffer->channels());
  EXPECT_EQ(200 * 3, image_buffer->row_stride());
  EXPECT_NE(nullptr, image_buffer->data());
  EXPECT_EQ(200 * 150 * 3, image_buffer->data_size());

  image_buffer->set_display_size(195, 145);
  EXPECT_EQ(195, image_buffer->display_width());
  EXPECT_EQ(145, image_buffer->display_height());

  // Contents should be zeroed.
  for (size_t i = 0; i < image_buffer->data_size(); ++i) {
    ASSERT_EQ(0, image_buffer->data()[i]) << "One or more data bytes differ";
  }
}

// Loads a compressed PNG from a file.
TEST(ImageBufferTest, LoadFilePNG) {
  // Load the image from memory with forced single channel.
  auto image_buffer =
      ImageBuffer::Load("xrtl/testing/testdata/image_file.png", 3);
  EXPECT_NE(nullptr, image_buffer);

  // Verify contents.
  EXPECT_TRUE(diffing::ImageDiffer::CompareImageBuffers(
      CreateGridRGB(3), image_buffer.get(), {}));
}

// Loads a compressed PNG from memory.
TEST(ImageBufferTest, LoadMemoryPNG) {
  // Load the image from memory with forced single channel.
  auto compressed_data =
      FileUtil::LoadFile("xrtl/testing/testdata/image_file.png");
  ASSERT_TRUE(compressed_data);
  EXPECT_FALSE(compressed_data->empty());
  auto image_buffer = ImageBuffer::Load(*compressed_data, 3);
  EXPECT_NE(nullptr, image_buffer);

  // Verify contents.
  EXPECT_TRUE(diffing::ImageDiffer::CompareImageBuffers(
      CreateGridRGB(3), image_buffer.get(), {}));
}

// Saves to a compressed PNG on the filesystem.
TEST(ImageBufferTest, SaveFilePNG) {
  // Create standard rgb.png in memory.
  auto image_buffer = CreateGridRGB(3);

  // Save the image with 1 channel.
  auto temp_file = FileUtil::MakeTempFile("rgb_full.png");
  ASSERT_TRUE(image_buffer->Save(temp_file.path()));

  // Compare against golden compressed data.
  EXPECT_TRUE(CompareCompressedData(
      *FileUtil::LoadFile("xrtl/testing/testdata/image_file.png"),
      *FileUtil::LoadFile(temp_file.path())));
}

// Saves to a compressed PNG in memory.
TEST(ImageBufferTest, SaveMemoryPNG) {
  // Create standard rgb.png in memory.
  auto image_buffer = CreateGridRGB(3);

  // Save the image as a PNG.
  auto compressed_data = image_buffer->Save();
  ASSERT_TRUE(compressed_data);

  // Compare against golden compressed data.
  EXPECT_TRUE(CompareCompressedData(
      *FileUtil::LoadFile("xrtl/testing/testdata/image_file.png"),
      *compressed_data));
}

// Tests clearing entire image contents.
TEST(ImageBufferTest, ClearAll) {
  auto image_buffer = ImageBuffer::Create(16, 8, 3);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Clear entire contents.
  image_buffer->Clear();
  // Verify entire contents cleared.
  for (size_t i = 0; i < image_buffer->data_size(); ++i) {
    ASSERT_EQ(0, image_buffer->data()[i]);
  }
}

// Tests clearing partial image contents.
TEST(ImageBufferTest, ClearSubregion) {
  auto image_buffer = ImageBuffer::Create(16, 8, 3);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Clear partial contents.
  image_buffer->Clear(2, 1, 8, 4);
  // Verify only the display area was cleared and the rest were untouched.
  for (int y = 0; y < image_buffer->data_height(); ++y) {
    for (int x = 0; x < image_buffer->data_width(); ++x) {
      bool in_fill_area = x >= 2 && x < 2 + 8 && y >= 1 && y < 1 + 4;
      size_t pixel_offset =
          y * image_buffer->row_stride() + x * image_buffer->channels();
      for (int c = 0; c < image_buffer->channels(); ++c) {
        ASSERT_EQ(in_fill_area ? 0 : 0xFE,
                  image_buffer->data()[pixel_offset + c]);
      }
    }
  }
}

// Tests filling entire image contents.
TEST(ImageBufferTest, FillAll) {
  auto image_buffer = ImageBuffer::Create(16, 8, 3);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Fill entire contents.
  const uint8_t kFillValue[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  image_buffer->Fill(kFillValue);
  // Verify entire contents filled.
  for (int y = 0; y < image_buffer->data_height(); ++y) {
    for (int x = 0; x < image_buffer->data_width(); ++x) {
      size_t pixel_offset =
          y * image_buffer->row_stride() + x * image_buffer->channels();
      for (int c = 0; c < image_buffer->channels(); ++c) {
        ASSERT_EQ(kFillValue[c], image_buffer->data()[pixel_offset + c]);
      }
    }
  }
}

// Tests filling partial image contents.
TEST(ImageBufferTest, FillSubregion) {
  auto image_buffer = ImageBuffer::Create(16, 8, 3);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Fill partial contents.
  const uint8_t kFillValue[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  image_buffer->Fill(2, 1, 8, 4, kFillValue);
  // Verify only the display area was cleared and the rest were untouched.
  for (int y = 0; y < image_buffer->data_height(); ++y) {
    for (int x = 0; x < image_buffer->data_width(); ++x) {
      bool in_fill_area = x >= 2 && x < 2 + 8 && y >= 1 && y < 1 + 4;
      size_t pixel_offset =
          y * image_buffer->row_stride() + x * image_buffer->channels();
      for (int c = 0; c < image_buffer->channels(); ++c) {
        ASSERT_EQ(in_fill_area ? kFillValue[c] : 0xFE,
                  image_buffer->data()[pixel_offset + c]);
      }
    }
  }
}

// Tests filling a specific channel in an entire image.
TEST(ImageBufferTest, FillChannelAll) {
  auto image_buffer = ImageBuffer::Create(16, 8, 3);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Fill entire contents of the G channel.
  image_buffer->FillChannel(1, 0xAA);
  // Verify entire contents filled.
  for (int y = 0; y < image_buffer->data_height(); ++y) {
    for (int x = 0; x < image_buffer->data_width(); ++x) {
      size_t pixel_offset =
          y * image_buffer->row_stride() + x * image_buffer->channels();
      for (int c = 0; c < image_buffer->channels(); ++c) {
        ASSERT_EQ(c == 1 ? 0xAA : 0xFE, image_buffer->data()[pixel_offset + c]);
      }
    }
  }
}

// Tests filling a specific channel in a partial image.
TEST(ImageBufferTest, FillChannelSubregion) {
  auto image_buffer = ImageBuffer::Create(16, 8, 3);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Fill partial contents of the G channel.
  image_buffer->FillChannel(2, 1, 8, 4, 1, 0xAA);
  // Verify entire contents filled.
  for (int y = 0; y < image_buffer->data_height(); ++y) {
    for (int x = 0; x < image_buffer->data_width(); ++x) {
      bool in_fill_area = x >= 2 && x < 2 + 8 && y >= 1 && y < 1 + 4;
      size_t pixel_offset =
          y * image_buffer->row_stride() + x * image_buffer->channels();
      for (int c = 0; c < image_buffer->channels(); ++c) {
        ASSERT_EQ((in_fill_area && c == 1) ? 0xAA : 0xFE,
                  image_buffer->data()[pixel_offset + c]);
      }
    }
  }
}

// Tests drawing a grid into an image buffer.
TEST(ImageBufferTest, DrawGridSubregion) {
  auto image_buffer = ImageBuffer::Create(6, 4, 2);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Draw grid into a subregion.
  const uint8_t kColorA[4] = {0xAA, 0xBB, 0, 0};
  const uint8_t kColorB[4] = {0x11, 0x22, 0, 0};
  image_buffer->set_display_size(5, 3);
  image_buffer->DrawGrid(1, kColorA, kColorB);
  // Verify grid drawn to the subregion of the image we care about.
  // Instead of replicating the grid logic here we just compare with an inlined
  // buffer.
  const uint8_t kExpectedValues[] = {
      0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0xFE, 0xFE,
      0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xFE, 0xFE,
      0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0xFE, 0xFE,
      0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
  };
  EXPECT_EQ(sizeof(kExpectedValues), image_buffer->data_size());
  EXPECT_EQ(0, std::memcmp(image_buffer->data(), kExpectedValues,
                           sizeof(kExpectedValues)));
}

// Tests drawing a grid using the packed uint32_t color form.
TEST(ImageBufferTest, DrawGridSubregionPacked) {
  auto image_buffer = ImageBuffer::Create(6, 4, 2);
  // Scribble entire contents.
  std::memset(image_buffer->data(), 0xFE, image_buffer->data_size());
  // Draw grid into a subregion.
  image_buffer->set_display_size(5, 3);
  image_buffer->DrawGrid(1, 0xAABB0000, 0x11220000);
  // Verify grid drawn to the subregion of the image we care about.
  // Instead of replicating the grid logic here we just compare with an inlined
  // buffer.
  const uint8_t kExpectedValues[] = {
      0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0xFE, 0xFE,
      0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xFE, 0xFE,
      0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0x11, 0x22, 0xAA, 0xBB, 0xFE, 0xFE,
      0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
  };
  EXPECT_EQ(sizeof(kExpectedValues), image_buffer->data_size());
  EXPECT_EQ(0, std::memcmp(image_buffer->data(), kExpectedValues,
                           sizeof(kExpectedValues)));
}

}  // namespace
}  // namespace testing
}  // namespace xrtl
