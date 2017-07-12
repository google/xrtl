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

#ifndef XRTL_GFX_VERTEX_FORMAT_H_
#define XRTL_GFX_VERTEX_FORMAT_H_

#include <cstdint>

#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"
#include "xrtl/gfx/pixel_format.h"

namespace xrtl {
namespace gfx {

// Each VertexFormat is defined by a unique ID combined with many packed flags.
// This allows us to encode the most commonly used information directly into the
// enum value and avoid indirections during data lookups. The unique ID allows
// us to use tables to map to internal formats.
class VertexFormat {
 public:
  VertexFormat() = default;
  constexpr VertexFormat(uint8_t unique_id, uint8_t packed_bytes_per_vertex,
                         ComponentFormat component_format,
                         uint8_t component_bits_x, uint8_t component_bits_y,
                         uint8_t component_bits_z, uint8_t component_bits_w)
      : unique_id_(unique_id),
        pad_(0),
        packed_bytes_per_vertex_(packed_bytes_per_vertex),
        component_format_(component_format),
        component_bits_x_(component_bits_x),
        component_bits_y_(component_bits_y),
        component_bits_z_(component_bits_z),
        component_bits_w_(component_bits_w) {}

  bool operator==(const VertexFormat& other) const {
    return unique_id_ == other.unique_id_;
  }
  bool operator!=(const VertexFormat& other) const { return !(*this == other); }

  // Returns the unique ID of the vertex format.
  // This byte value can be used in lookup tables as no other vertex format in
  // the system will have it.
  // Values of 0 always indicate 'undefined' formats.
  constexpr int unique_id() const { return unique_id_; }
  constexpr operator int() const { return unique_id_; }

  // Returns the total bytes used by an attribute of this format per vertex.
  size_t data_size() const { return packed_bytes_per_vertex_; }

  // Returns the format components are stored in.
  constexpr ComponentFormat component_format() const {
    return component_format_;
  }

  // Returns the total number of components in the vertex format.
  // For example, X8Y8Z8 would return 3.
  constexpr int component_count() const {
    return ((component_bits_x_ != 0) ? 1 : 0) +
           ((component_bits_y_ != 0) ? 1 : 0) +
           ((component_bits_z_ != 0) ? 1 : 0) +
           ((component_bits_w_ != 0) ? 1 : 0);
  }

  // Returns the size, in bits, of each component of the format.
  // A size of 0 indicates the component is not present in the format.
  constexpr int component_bits_x() const { return component_bits_x_; }
  constexpr int component_bits_y() const { return component_bits_y_; }
  constexpr int component_bits_z() const { return component_bits_z_; }
  constexpr int component_bits_w() const { return component_bits_w_; }

 private:
  uint8_t unique_id_;
  uint8_t pad_;
  uint8_t packed_bytes_per_vertex_;
  ComponentFormat component_format_;
  uint8_t component_bits_x_;
  uint8_t component_bits_y_;
  uint8_t component_bits_z_;
  uint8_t component_bits_w_;
};
static_assert(sizeof(VertexFormat) == sizeof(uint64_t),
              "Vertex format must be representable as a uint64_t");

// Vertex formats that are supported throughout the system.
// Some of these are only available for use in the graphics system on certain
// platforms. Validate formats before using, or stick to commonly supported
// formats for safety.
//
// Vulkan formats:
// https://vulkan.lunarg.com/doc/view/1.0.30.0/linux/vkspec.chunked/ch31s03.html
//
// Metal formats:
// https://developer.apple.com/reference/metal/mtlvertexformat
//
// D3D formats:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
namespace VertexFormats {

// clang-format off

//                                    component_bits_w -----------------------------------------------+
//                                    component_bits_z -------------------------------------------+   |
//                                    component_bits_y ---------------------------------------+   |   |
//                                    component_bits_x -----------------------------------+   |   |   |
//                                    component_format --------+                          |   |   |   |
//                             packed_bytes_per_vertex -----+  |                          |   |   |   |
//                                           unique_id +    |  |                          |   |   |   |
//                                                     |    |  |                          |   |   |   |
//                                                     v    v  v                          v   v   v   v
constexpr static VertexFormat kUndefined           = {  0,  0, ComponentFormat::kSFloat,  0,  0,  0,  0};

constexpr static VertexFormat kX8UNorm             = {  1,  1, ComponentFormat::kUNorm,   8,  0,  0,  0};
constexpr static VertexFormat kX8SNorm             = {  2,  1, ComponentFormat::kSNorm,   8,  0,  0,  0};
constexpr static VertexFormat kX8UInt              = {  3,  1, ComponentFormat::kUInt,    8,  0,  0,  0};
constexpr static VertexFormat kX8SInt              = {  4,  1, ComponentFormat::kSInt,    8,  0,  0,  0};

constexpr static VertexFormat kX8Y8UNorm           = {  5,  2, ComponentFormat::kUNorm,   8,  8,  0,  0};
constexpr static VertexFormat kX8Y8SNorm           = {  6,  2, ComponentFormat::kSNorm,   8,  8,  0,  0};
constexpr static VertexFormat kX8Y8UInt            = {  7,  2, ComponentFormat::kUInt,    8,  8,  0,  0};
constexpr static VertexFormat kX8Y8SInt            = {  8,  2, ComponentFormat::kSInt,    8,  8,  0,  0};

constexpr static VertexFormat kX8Y8Z8UNorm         = {  9,  3, ComponentFormat::kUNorm,   8,  8,  8,  0};
constexpr static VertexFormat kX8Y8Z8SNorm         = { 10,  3, ComponentFormat::kSNorm,   8,  8,  8,  0};
constexpr static VertexFormat kX8Y8Z8UInt          = { 11,  3, ComponentFormat::kUInt,    8,  8,  8,  0};
constexpr static VertexFormat kX8Y8Z8SInt          = { 12,  3, ComponentFormat::kSInt,    8,  8,  8,  0};

constexpr static VertexFormat kX8Y8Z8W8UNorm       = { 13,  4, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static VertexFormat kX8Y8Z8W8SNorm       = { 14,  4, ComponentFormat::kSNorm,   8,  8,  8,  8};
constexpr static VertexFormat kX8Y8Z8W8UInt        = { 15,  4, ComponentFormat::kUInt,    8,  8,  8,  8};
constexpr static VertexFormat kX8Y8Z8W8SInt        = { 16,  4, ComponentFormat::kSInt,    8,  8,  8,  8};

constexpr static VertexFormat kW2X10Y10Z10UNorm    = { 17,  4, ComponentFormat::kUNorm,  10, 10, 10,  2};
constexpr static VertexFormat kW2X10Y10Z10SNorm    = { 18,  4, ComponentFormat::kSNorm,  10, 10, 10,  2};

constexpr static VertexFormat kX16UNorm            = { 19,  2, ComponentFormat::kUNorm,  16,  0,  0,  0};
constexpr static VertexFormat kX16SNorm            = { 20,  2, ComponentFormat::kSNorm,  16,  0,  0,  0};
constexpr static VertexFormat kX16UInt             = { 21,  2, ComponentFormat::kUInt,   16,  0,  0,  0};
constexpr static VertexFormat kX16SInt             = { 22,  2, ComponentFormat::kSInt,   16,  0,  0,  0};
constexpr static VertexFormat kX16SFloat           = { 23,  2, ComponentFormat::kSFloat, 16,  0,  0,  0};

constexpr static VertexFormat kX16Y16UNorm         = { 24,  4, ComponentFormat::kUNorm,  16, 16,  0,  0};
constexpr static VertexFormat kX16Y16SNorm         = { 25,  4, ComponentFormat::kSNorm,  16, 16,  0,  0};
constexpr static VertexFormat kX16Y16UInt          = { 26,  4, ComponentFormat::kUInt,   16, 16,  0,  0};
constexpr static VertexFormat kX16Y16SInt          = { 27,  4, ComponentFormat::kSInt,   16, 16,  0,  0};
constexpr static VertexFormat kX16Y16SFloat        = { 28,  4, ComponentFormat::kSFloat, 16, 16,  0,  0};

constexpr static VertexFormat kX16Y16Z16UNorm      = { 29,  6, ComponentFormat::kUNorm,  16, 16, 16,  0};
constexpr static VertexFormat kX16Y16Z16SNorm      = { 30,  6, ComponentFormat::kSNorm,  16, 16, 16,  0};
constexpr static VertexFormat kX16Y16Z16UInt       = { 31,  6, ComponentFormat::kUInt,   16, 16, 16,  0};
constexpr static VertexFormat kX16Y16Z16SInt       = { 32,  6, ComponentFormat::kSInt,   16, 16, 16,  0};
constexpr static VertexFormat kX16Y16Z16SFloat     = { 33,  6, ComponentFormat::kSFloat, 16, 16, 16,  0};

constexpr static VertexFormat kX16Y16Z16W16UNorm   = { 34,  8, ComponentFormat::kUNorm,  16, 16, 16, 16};
constexpr static VertexFormat kX16Y16Z16W16SNorm   = { 35,  8, ComponentFormat::kSNorm,  16, 16, 16, 16};
constexpr static VertexFormat kX16Y16Z16W16UInt    = { 36,  8, ComponentFormat::kUInt,   16, 16, 16, 16};
constexpr static VertexFormat kX16Y16Z16W16SInt    = { 37,  8, ComponentFormat::kSInt,   16, 16, 16, 16};
constexpr static VertexFormat kX16Y16Z16W16SFloat  = { 38,  8, ComponentFormat::kSFloat, 16, 16, 16, 16};

constexpr static VertexFormat kX32UInt             = { 39,  4, ComponentFormat::kUInt,   32,  0,  0,  0};
constexpr static VertexFormat kX32SInt             = { 40,  4, ComponentFormat::kSInt,   32,  0,  0,  0};
constexpr static VertexFormat kX32SFloat           = { 41,  4, ComponentFormat::kSFloat, 32,  0,  0,  0};

constexpr static VertexFormat kX32Y32UInt          = { 42,  8, ComponentFormat::kUInt,   32, 32,  0,  0};
constexpr static VertexFormat kX32Y32SInt          = { 43,  8, ComponentFormat::kSInt,   32, 32,  0,  0};
constexpr static VertexFormat kX32Y32SFloat        = { 44,  8, ComponentFormat::kSFloat, 32, 32,  0,  0};

constexpr static VertexFormat kX32Y32Z32UInt       = { 45, 12, ComponentFormat::kUInt,   32, 32, 32,  0};
constexpr static VertexFormat kX32Y32Z32SInt       = { 46, 12, ComponentFormat::kSInt,   32, 32, 32,  0};
constexpr static VertexFormat kX32Y32Z32SFloat     = { 47, 12, ComponentFormat::kSFloat, 32, 32, 32,  0};

constexpr static VertexFormat kX32Y32Z32W32UInt    = { 48, 16, ComponentFormat::kUInt,   32, 32, 32, 32};
constexpr static VertexFormat kX32Y32Z32W32SInt    = { 49, 16, ComponentFormat::kSInt,   32, 32, 32, 32};
constexpr static VertexFormat kX32Y32Z32W32SFloat  = { 50, 16, ComponentFormat::kSFloat, 32, 32, 32, 32};

// Something missing from this list? Append only! IDs must be dense and you'll
// likely get some compile warnings about tables that need updating.
//
// Instructions:
// - Append a new VertexFormat to the end of the table above with a new ID.
// - Match the formatting; lint won't do it for you.
// - Compile with --keep_going and wait for all the errors (maybe none?!).

// clang-format on

}  // namespace VertexFormats

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_VERTEX_FORMAT_H_
