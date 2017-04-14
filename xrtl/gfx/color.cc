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

#include "xrtl/gfx/color.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "xrtl/base/math.h"

namespace xrtl {
namespace gfx {
namespace color {

bool FromString(const char* string_value, rgba8_t* out_value) {
  // We do this piecewise so we can get the byte order right without doing
  // swapping tricks.
  size_t length = std::strlen(string_value);
  if (length == 7 || length == 9) {
    // Strip # if present.
    if (string_value[0] == '#') {
      ++string_value;
      --length;
    }
  }
  if (length != 6 && length != 8) {
    return false;
  }
  char part[3] = {0};
  part[0] = string_value[0];
  part[1] = string_value[1];
  out_value->r = strtoul(part, nullptr, 16);  // NOLINT
  part[0] = string_value[2];
  part[1] = string_value[3];
  out_value->g = strtoul(part, nullptr, 16);  // NOLINT
  part[0] = string_value[4];
  part[1] = string_value[5];
  out_value->b = strtoul(part, nullptr, 16);  // NOLINT
  if (length == 8) {
    part[0] = string_value[6];
    part[1] = string_value[7];
    out_value->a = strtoul(part, nullptr, 16);  // NOLINT
  } else {
    out_value->a = 0xFF;
  }
  return true;
}

rgba8_t FromString(const char* string_value) {
  rgba8_t result;
  if (FromString(string_value, &result)) {
    return result;
  } else {
    return kTransparent;
  }
}

rgba8_t Lerp(const rgba8_t& color_1, const rgba8_t& color_2, float t) {
  // Inspired from this: https://www.shadertoy.com/view/lsdGzN
  // Tweaked to be a bit faster and preserve colors at boundaries.
  if (color_1 == color_2) {
    return color_2;
  }

  // Fast path for alpha only.
  uint8_t final_a = static_cast<uint8_t>(math::Clamp(
      static_cast<uint32_t>(color_1.a + (color_2.a - color_1.a) * t), 0u,
      255u));
  if ((color_1 & 0x00FFFFFF) == (color_2 & 0x00FFFFFF)) {
    rgba8_t result = color_2;
    result.a = final_a;
    return result;
  }

  // Bring each component into [0-1].
  float start_r = color_1.r / static_cast<float>(0xFF);
  float start_g = color_1.g / static_cast<float>(0xFF);
  float start_b = color_1.b / static_cast<float>(0xFF);
  float end_r = color_2.r / static_cast<float>(0xFF);
  float end_g = color_2.g / static_cast<float>(0xFF);
  float end_b = color_2.b / static_cast<float>(0xFF);

  // Magic!
  float max_1 = std::max(start_r, std::max(start_g, start_b));
  float max_2 = std::max(end_r, std::max(end_g, end_b));
  float m = (max_1 + max_2) / 2.0f;
  float avg_r = (start_r + end_r) / 2.0f;
  float avg_g = (start_g + end_g) / 2.0f;
  float avg_b = (start_b + end_b) / 2.0f;
  float min_avg = std::min(avg_r, std::min(avg_g, avg_b));
  float d = 2.0f * std::abs(t - 0.5f) * min_avg;
  // Note we try to prevent NaNs.
  float inv_d = 1.0f / (d == 1.0f ? (1.0f - 0.9999f) : (1.0f - d));
  float c_r = (avg_r - d) * inv_d;
  float c_g = (avg_g - d) * inv_d;
  float c_b = (avg_b - d) * inv_d;
  float m_div = std::max(c_r, std::max(c_g, c_b));
  float m_scale = m_div ? m / m_div : m;
  c_r *= m_scale;
  c_g *= m_scale;
  c_b *= m_scale;
  float s = t >= 0.5f ? 1.0f : 0.0f;
  float out_r = (1.0f - s) * math::Lerp(start_r, c_r, t * 2.0f) +
                s * math::Lerp(c_r, end_r, t * 2.0f - 1.0f);
  float out_g = (1.0f - s) * math::Lerp(start_g, c_g, t * 2.0f) +
                s * math::Lerp(c_g, end_g, t * 2.0f - 1.0f);
  float out_b = (1.0f - s) * math::Lerp(start_b, c_b, t * 2.0f) +
                s * math::Lerp(c_b, end_b, t * 2.0f - 1.0f);

  // Bring back into [0-255].
  float final_r = std::floor(math::Clamp(out_r * 255.0f, 0.0f, 255.0f));
  float final_g = std::floor(math::Clamp(out_g * 255.0f, 0.0f, 255.0f));
  float final_b = std::floor(math::Clamp(out_b * 255.0f, 0.0f, 255.0f));
  return rgba8_t(static_cast<uint8_t>(final_r), static_cast<uint8_t>(final_g),
                 static_cast<uint8_t>(final_b), static_cast<uint8_t>(final_a));
}

}  // namespace color
}  // namespace gfx
}  // namespace xrtl
