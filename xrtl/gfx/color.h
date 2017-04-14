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

#ifndef XRTL_GFX_COLOR_H_
#define XRTL_GFX_COLOR_H_

#include <string>

#include "xrtl/base/macros.h"

namespace xrtl {
namespace gfx {

// A 32-bit RGB color with alpha.
struct rgba8_t {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t a = 0;

  rgba8_t() = default;
  constexpr rgba8_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
      : r(r), g(g), b(b), a(a) {}
  constexpr explicit rgba8_t(uint32_t value)
      : r(static_cast<uint8_t>((value >> 0) & 0xFF)),
        g(static_cast<uint8_t>((value >> 8) & 0xFF)),
        b(static_cast<uint8_t>((value >> 16) & 0xFF)),
        a(static_cast<uint8_t>((value >> 24) & 0xFF)) {}

  // Returns the color packed into AABBGGRR form.
  constexpr operator uint32_t() const {
    return ((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((g & 0xFF) << 8) |
           (r & 0xFF);
  }

  // Returns true if the color value is fully transparent.
  constexpr bool is_transparent() const { return a == 0; }

  // Returns true if the color value is fully opaque.
  constexpr bool is_opaque() const { return a == 0xFF; }

  bool operator==(const rgba8_t& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }
  bool operator!=(const rgba8_t& other) const { return !(*this == other); }
};

// TODO(benvanik): YUV/YUVA, etc.

namespace color {

// Some commonly used colors. See xrtl::ui::SystemMetrics for system colors.
constexpr static rgba8_t kTransparent = {0, 0, 0, 0};
constexpr static rgba8_t kBlack = {0, 0, 0, 0xFF};
constexpr static rgba8_t kWhite = {0xFF, 0xFF, 0xFF, 0xFF};
constexpr static rgba8_t kRed = {0xFF, 0, 0, 0xFF};
constexpr static rgba8_t kGreen = {0, 0xFF, 0, 0xFF};
constexpr static rgba8_t kBlue = {0, 0, 0xFF, 0xFF};

// Parses a color from a string value containing the hex value as
// either '#RRGGBB[AA]' or 'RRGGBB[AA]'. If alpha is omitted it will be 0xFF.
// Returns true if the color parsed successfully.
bool FromString(const char* string_value, rgba8_t* out_value);

// Parses a color from a string value containing the hex value as
// either '#RRGGBB[AA]' or 'RRGGBB[AA]'. If alpha is omitted it will be 0xFF.
// Returns kTransparent if the color could not be parsed.
rgba8_t FromString(const char* string_value);

// Interpolates between two colors in a way that prevents greyness.
// Returns a color where if t = 0 is all color_1 and t = 1 is all color_2.
rgba8_t Lerp(const rgba8_t& color_1, const rgba8_t& color_2, float t);

}  // namespace color

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_COLOR_H_
