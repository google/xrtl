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

#include <stb_image.h>
#include <stb_image_write.h>

#include <string>
#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/base/tracing.h"
#include "xrtl/testing/file_util.h"

// Not exposed from stb_image_write:
extern unsigned char* stbi_write_png_to_mem(unsigned char* pixels,
                                            int stride_bytes, int x, int y,
                                            int n, int* out_len);

namespace xrtl {
namespace testing {

ImageBuffer::ImageBuffer() = default;

ImageBuffer::~ImageBuffer() = default;

std::unique_ptr<ImageBuffer> ImageBuffer::Create(int data_width,
                                                 int data_height,
                                                 int channels) {
  WTF_SCOPE0("ImageBuffer#Create");
  CHECK_GT(data_width, 0);
  CHECK_GT(data_height, 0);
  CHECK_GT(channels, 0);

  auto image_buffer = absl::WrapUnique(new ImageBuffer());
  image_buffer->data_width_ = data_width;
  image_buffer->data_height_ = data_height;
  image_buffer->display_width_ = data_width;
  image_buffer->display_height_ = data_height;
  image_buffer->channels_ = channels;

  // We'll allocate normal heap memory and free it ourselves.
  image_buffer->data_ = {
      reinterpret_cast<uint8_t*>(std::calloc(1, image_buffer->data_size())),
      [](uint8_t* data) { std::free(data); }};

  return image_buffer;
}

std::unique_ptr<ImageBuffer> ImageBuffer::Load(const void* compressed_data,
                                               size_t compressed_data_length,
                                               int channels) {
  WTF_SCOPE0("ImageBuffer#Load");
  CHECK_NE(compressed_data, nullptr);
  CHECK_GT(compressed_data_length, 0);
  CHECK_GT(channels, 0);

  auto image_buffer = absl::WrapUnique(new ImageBuffer());

  // Load the file with stbimage.
  int actual_channels = 0;
  image_buffer->data_ = {
      stbi_load_from_memory(
          static_cast<const stbi_uc*>(compressed_data),
          static_cast<int>(compressed_data_length), &image_buffer->data_width_,
          &image_buffer->data_height_, &actual_channels, channels),
      [](uint8_t* data) { stbi_image_free(data); }};
  if (!image_buffer->data_) {
    LOG(ERROR) << "Failed to decompress image buffer";
    return nullptr;
  }
  image_buffer->channels_ = channels ? channels : actual_channels;

  // Default display dimensions.
  image_buffer->display_width_ = image_buffer->data_width();
  image_buffer->display_height_ = image_buffer->data_height();
  return image_buffer;
}

std::unique_ptr<ImageBuffer> ImageBuffer::Load(absl::string_view path,
                                               int channels) {
  WTF_SCOPE0("ImageBuffer#Load");
  CHECK_GT(channels, 0);

  auto image_buffer = absl::WrapUnique(new ImageBuffer());

  // Lookup the file path in runfiles. This will adjust for any test environment
  // path manipulation that needs to take place.
  absl::optional<std::string> resolved_path_opt = FileUtil::ResolvePath(path);
  if (!resolved_path_opt) {
    LOG(ERROR) << "Image file not found: " << path;
    return nullptr;
  }
  std::string resolved_path = resolved_path_opt.value();

  // Load the file with stbimage.
  int actual_channels = 0;
  image_buffer->data_ = {
      stbi_load(resolved_path.c_str(), &image_buffer->data_width_,
                &image_buffer->data_height_, &actual_channels, channels),
      [](uint8_t* data) { stbi_image_free(data); }};
  if (!image_buffer->data_) {
    LOG(ERROR) << "Failed to load and decompress image buffer at " << path
               << " (resolved: " << resolved_path << ")";
    return nullptr;
  }
  image_buffer->channels_ = channels ? channels : actual_channels;

  // Default display dimensions.
  image_buffer->display_width_ = image_buffer->data_width();
  image_buffer->display_height_ = image_buffer->data_height();
  return image_buffer;
}

absl::optional<std::vector<uint8_t>> ImageBuffer::Save(int channels) {
  WTF_SCOPE0("ImageBuffer#Save");
  CHECK_GT(channels, 0);

  // Compress data.
  int compressed_data_length = 0;
  uint8_t* compressed_data_ptr = stbi_write_png_to_mem(
      data(), row_stride(), display_width(), display_height(), channels,
      &compressed_data_length);
  if (!compressed_data_ptr) {
    LOG(ERROR) << "Failed to compress PNG data";
    return absl::nullopt;
  }

  // Copy to a vector. We could use a different type that was no-copy if we
  // wanted.
  std::vector<uint8_t> compressed_data;
  compressed_data.resize(compressed_data_length);
  std::memcpy(compressed_data.data(), compressed_data_ptr,
              compressed_data_length);

  std::free(compressed_data_ptr);
  return compressed_data;
}

bool ImageBuffer::Save(absl::string_view path, int channels) {
  WTF_SCOPE0("ImageBuffer#Save");
  CHECK_GT(channels, 0);

  // Resolve path to our real output location.
  // TODO(benvanik): something real here using env vars? assume caller has
  //                 done the right thing?
  std::string resolved_path = std::string(path);

  // Compress data and write to the target file path.
  if (stbi_write_png(resolved_path.c_str(), display_width(), display_height(),
                     channels, data(), row_stride()) == 0) {
    LOG(ERROR) << "Failed to compress PNG and write data to file " << path
               << " (resolved: " << resolved_path << ")";
    return false;
  }

  return true;
}

void ImageBuffer::Clear(int x, int y, int width, int height) {
  WTF_SCOPE0("ImageBuffer#Clear");
  uint8_t kZeros[4] = {0, 0, 0, 0};
  Fill(x, y, width, height, kZeros);
}

void ImageBuffer::Fill(int x, int y, int width, int height,
                       const uint8_t channel_values[4]) {
  WTF_SCOPE0("ImageBuffer#Fill");
  CHECK_LE(x + width, data_width_);
  CHECK_LE(y + height, data_height_);
  uint8_t* data = this->data();
  size_t row_stride = this->row_stride();
  for (int py = y; py < y + height; ++py) {
    for (int px = x; px < x + width; ++px) {
      size_t pixel_offset = py * row_stride + px * channels_;
      for (int c = 0; c < channels_; ++c) {
        data[pixel_offset + c] = channel_values[c];
      }
    }
  }
}

void ImageBuffer::FillChannel(int x, int y, int width, int height, int channel,
                              uint8_t channel_value) {
  WTF_SCOPE0("ImageBuffer#FillChannel");
  CHECK_LE(x + width, data_width_);
  CHECK_LE(y + height, data_height_);
  CHECK_LT(channel, channels_);
  uint8_t* data = this->data();
  size_t row_stride = this->row_stride();
  for (int py = y; py < y + height; ++py) {
    for (int px = x; px < x + width; ++px) {
      size_t pixel_offset = py * row_stride + px * channels_;
      data[pixel_offset + channel] = channel_value;
    }
  }
}

void ImageBuffer::DrawGrid(int cell_size, const uint8_t color_a[4],
                           const uint8_t color_b[4]) {
  WTF_SCOPE0("ImageBuffer#DrawGrid");
  CHECK_GT(cell_size, 0);
  uint8_t* data = this->data();
  size_t row_stride = this->row_stride();
  for (int py = 0; py < display_height_; ++py) {
    bool which_y = (py / cell_size) % 2 == 0;
    for (int px = 0; px < display_width_; ++px) {
      bool which_x = (px / cell_size) % 2 == 0;
      const uint8_t* color = which_x ^ which_y ? color_b : color_a;
      size_t pixel_offset = py * row_stride + px * channels_;
      for (int c = 0; c < channels_; ++c) {
        data[pixel_offset + c] = color[c];
      }
    }
  }
}

void ImageBuffer::DrawGrid(int cell_size, uint32_t color_a, uint32_t color_b) {
  uint8_t color_a_bytes[4] = {
      static_cast<uint8_t>((color_a >> 24) & 0xFF),
      static_cast<uint8_t>((color_a >> 16) & 0xFF),
      static_cast<uint8_t>((color_a >> 8) & 0xFF),
      static_cast<uint8_t>((color_a >> 0) & 0xFF),
  };
  uint8_t color_b_bytes[4] = {
      static_cast<uint8_t>((color_b >> 24) & 0xFF),
      static_cast<uint8_t>((color_b >> 16) & 0xFF),
      static_cast<uint8_t>((color_b >> 8) & 0xFF),
      static_cast<uint8_t>((color_b >> 0) & 0xFF),
  };
  DrawGrid(cell_size, color_a_bytes, color_b_bytes);
}

}  // namespace testing
}  // namespace xrtl
