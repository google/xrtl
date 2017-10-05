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

#ifndef XRTL_TESTING_IMAGE_BUFFER_H_
#define XRTL_TESTING_IMAGE_BUFFER_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace xrtl {
namespace testing {

// A simple uncompressed image buffer wrapper.
// Use this in tests to create synthetic images and load/save images in a form
// compatible with the other testing infrastructure.
//
// This has minimal dependencies to make it less flakey when used as a core
// testing library. It's not fast, or efficient, and doesn't support most image
// formats, but it should work fine for common RGB/RGBA images.
//
// The buffers are stored as PNGs compressed with stbimage, which does not
// produce very small files.
class ImageBuffer {
 public:
  // Creates a new image buffer with the specified dimensions.
  // The contents of the image buffer will be all zeros.
  static std::unique_ptr<ImageBuffer> Create(int data_width, int data_height,
                                             int channels);

  // Loads an image buffer from the given compressed PNG data.
  // The resulting image will contain the specified number of channels
  // regardless of what the input data has.
  static std::unique_ptr<ImageBuffer> Load(const void* compressed_data,
                                           size_t compressed_data_length,
                                           int channels);
  static std::unique_ptr<ImageBuffer> Load(
      const std::vector<uint8_t>& compressed_data, int channels) {
    return Load(compressed_data.data(), compressed_data.size(), channels);
  }

  // Loads an image buffer from the given file path.
  // The path may be either runfiles-relative or absolute. The resulting image
  // will contain the specified number of channels regardless of what the input
  // file has.
  static std::unique_ptr<ImageBuffer> Load(absl::string_view path,
                                           int channels);

  ~ImageBuffer();

  // Dimensions of the image data buffer in pixels.
  // Note that the display dimensions may be less than this.
  int data_width() const { return data_width_; }
  int data_height() const { return data_height_; }

  // Dimensions of the valid image data in pixels.
  // This is <= the data dimensions.
  int display_width() const { return display_width_; }
  int display_height() const { return display_height_; }
  void set_display_size(int display_width, int display_height) {
    display_width_ = std::min(display_width, data_width_);
    display_height_ = std::min(display_height, data_height_);
  }

  // The number of color channels in the image buffer.
  // 1=Y, 2=YA, 3=RGB, 4=RGBA (Y is monochrome color).
  int channels() const { return channels_; }

  // The stride of each row in bytes.
  size_t row_stride() const { return data_width_ * channels_; }

  // The raw image buffer data.
  // The data is row_stride * data_height bytes, with only the area defined by
  // display_width/display_height being considered valid.
  const uint8_t* data() const { return data_.get(); }
  uint8_t* data() { return data_.get(); }
  // Total size of the image buffer data in bytes.
  size_t data_size() const { return row_stride() * data_height(); }

  // Saves the image buffer to a heap memory buffer.
  // Returns a compressed PNG if successful.
  absl::optional<std::vector<uint8_t>> Save() { return Save(channels_); }
  absl::optional<std::vector<uint8_t>> Save(int channels);

  // Saves the image buffer to the given file path.
  // The file will be encoded in PNG format with either the number of channels
  // in the buffer or the specified override, if provided.
  // Returns true if the image buffer encoded and was written successfully.
  bool Save(absl::string_view path) { return Save(path, channels_); }
  bool Save(absl::string_view path, int channels);

  // Clears the image buffer to zero.
  // Only the area specified by the display dimensions will be cleared.
  void Clear() { Clear(0, 0, display_width(), display_height()); }

  // Clears a subregion of the image buffer to zero.
  void Clear(int x, int y, int width, int height);

  // Fills the image buffer with the given channel values.
  // Only the area specified by the display dimensions will be filled.
  void Fill(const uint8_t channel_values[4]) {
    Fill(0, 0, display_width(), display_height(), channel_values);
  }

  // Fills a subregion of the image buffer with the given channel values.
  void Fill(int x, int y, int width, int height,
            const uint8_t channel_values[4]);

  // Fills the specified channel in the image buffer with the given value.
  // Only the area specified by the display dimensions will be filled.
  void FillChannel(int channel, uint8_t channel_value) {
    FillChannel(0, 0, display_width(), display_height(), channel,
                channel_value);
  }

  // Fills the specified channel in a subregion of the image buffer with the
  // given value.
  void FillChannel(int x, int y, int width, int height, int channel,
                   uint8_t channel_value);

  // Draws a grid into the image buffer with the given cell size and colors.
  // Only the area specified by the display dimensions will be filled.
  // Colors are specified as {r, g, b, a}.
  void DrawGrid(int cell_size, const uint8_t color_a[4],
                const uint8_t color_b[4]);
  // Draws a grid into the image buffer with the given cell size and colors.
  // Only the area specified by the display dimensions will be filled.
  // Colors are specified as 0xRRGGBBAA.
  void DrawGrid(int cell_size, uint32_t color_a, uint32_t color_b);

 private:
  ImageBuffer();

  using ImageDataPtr = std::unique_ptr<uint8_t, void (*)(uint8_t*)>;

  int data_width_ = 0;
  int data_height_ = 0;
  int display_width_ = 0;
  int display_height_ = 0;
  int channels_ = 0;
  ImageDataPtr data_ = {nullptr, nullptr};
};

}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_IMAGE_BUFFER_H_
