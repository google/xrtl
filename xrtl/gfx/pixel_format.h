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

#ifndef XRTL_GFX_PIXEL_FORMAT_H_
#define XRTL_GFX_PIXEL_FORMAT_H_

#include <cstdint>

#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"

namespace xrtl {
namespace gfx {

// Defines how pixel data is packed and represented.
enum class PixelPacking : uint8_t {
  // Bit indicating the format is flexibly defined, which is often used to query
  // for preferred formats. No storage or sampling can be performed on flexible
  // types and they must always be resolved to a concrete type first.
  //
  // Component sizes are treated as recommendations when querying for compatible
  // formats. Packing format is matched directly.
  kFlexibleMask = 1 << 6,

  // Bit indicating the data packing is compressed in some block format.
  kCompressedMask = 1 << 7,

  // Format is packed in a system-dependent way that cannot be queried.
  // This is commonly used for tiled resources or intermediate frame buffers
  // that may never exist off-chip.
  kOpaque = 0,

  // Format is packed as a depth and/or stencil buffer.
  // Component 0 is depth size and component 1 is stencil size.
  kDepthStencil = 1,
  // Like kDepthStencil but allowed to be of at least the component sizes
  // provided.
  kFlexibleDepthStencil = kDepthStencil | kFlexibleMask,

  // Format is uncompressed 'normal' data, such as RGBA bytes.
  kUncompressed = 2,
  // Like kUncompressed but allowed to be of at least the component sizes
  // provided.
  kFlexibleUncompressed = kUncompressed | kFlexibleMask,

  // BC ('Block Compression') formats.
  // https://msdn.microsoft.com/en-us/library/windows/desktop/hh308955(v=vs.85).aspx
  // https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#appendix-compressedtex-bc
  kBC1 = 3 | kCompressedMask,  // 4x4 block = 8 bytes
  kBC2 = 4 | kCompressedMask,  // 4x4 block = 16 bytes
  kBC3 = 5 | kCompressedMask,  // 4x4 block = 16 bytes
  kBC4 = 6 | kCompressedMask,  // 4x4 block = 8 bytes
  kBC5 = 7 | kCompressedMask,  // 4x4 block = 16 bytes
  kBC6 = 8 | kCompressedMask,  // 4x4 block = 16 bytes
  kBC7 = 9 | kCompressedMask,  // 4x4 block = 16 bytes

  // ETC/EAC block-compressed format.
  // https://en.wikipedia.org/wiki/Ericsson_Texture_Compression
  // https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#appendix-compressedtex-etc2
  kEtc1 = 10 | kCompressedMask,  // 4x4 block = 8 bytes
  kEtc2 = 11 | kCompressedMask,  // 4x4 block = 8 or 16 bytes
  kEac = 12 | kCompressedMask,   // 4x4 block = 16 bytes

  // ASTC block-compressed format.
  // https://en.wikipedia.org/wiki/Adaptive_Scalable_Texture_Compression
  // https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#appendix-compressedtex-astc
  kAstc = 13 | kCompressedMask,

  // PVRTC block-compressed format as used by PowerVR chipsets, commonly iOS.
  // https://en.wikipedia.org/wiki/PVRTC
  kPvrtc = 14 | kCompressedMask,
};

// Component data type used in pixel formats.
// This defines how the values are sampled in shaders. The widths and range of
// each type is based on the component widths
enum class ComponentFormat : uint8_t {
  // Signed floating-point value.
  // 16-bit floating-point formats use half-precision (s10e5 format).
  // 32-bit floating-point formats use IEEE-754 single-precision (s23e8 format).
  //
  // Suffixes:
  //  D3D12: _FLOAT
  //  Metal: Float
  //  Vulkan: _SFLOAT
  kSFloat = 0,

  // Unsigned floating-point value, used by some packed formats.
  //
  // Suffixes:
  //  D3D12: _FLOAT
  //  Metal: Float
  //  Vulkan: _UFLOAT
  kUFloat = 1,

  // Signed normalized integer of some number of bits.
  // When sampled in shaders this is normalzed to [-1.0, 1.0].
  //
  // Example:
  //  kSNorm with size 2 as [-2, -1, 0, 1] is sampled as [-1.0, ..., 0.0, 1.0].
  //
  // Suffixes:
  //  D3D12: _SNORM
  //  Metal: Snorm
  //  Vulkan: _SNORM
  kSNorm = 2,

  // Unsigned normalized integer of some number of bits.
  // When sampled in shaders this is normalzed to [0.0, 1.0].
  //
  // Example:
  //  kUNorm with size 2 as [0, 1, 2, 3] is sampled as [0.0, 1/3.0, 2/3.0, 1.0].
  //
  // Suffixes:
  //  D3D12: _UNORM
  //  Metal: Unorm
  //  Vulkan: _UNORM
  kUNorm = 3,

  // Signed integer of some number of bits.
  // This is sampled in shaders with no normalization.
  //
  // Example:
  //  kSInt with size 2 is [-2, -1, 0, 1].
  //
  // Suffixes:
  //  D3D12: _SINT
  //  Metal: Sint
  //  Vulkan: _SINT
  kSInt = 4,

  // Unsigned integer of some number of bits.
  // This is sampled in shaders with no normalization.
  //
  // Example:
  //  kUInt with size 2 is [0, 1, 2, 3].
  //
  // Suffixes:
  //  D3D12: _UINT
  //  Metal: Uint
  //  Vulkan: _UINT
  kUInt = 5,

  // Signed floating-point value that is mapped through the sRGB ramp.
  // Values of 0.0 will sample as 0.0 and 1.0 as 1.0, but the values inbetween
  // will be modified. Alpha channels, if present, will never be adjusted.
  //
  // Suffixes:
  //  D3D12: _SRGB
  //  Metal: _sRGB/_srgb
  //  Vulkan: _SRGB
  kSrgb = 6,
};

// Each PixelFormat is defined by a unique ID combined with many packed flags.
// This allows us to encode the most commonly used information directly into the
// enum value and avoid indirections during data lookups. The unique ID allows
// us to use tables to map to internal formats.
class PixelFormat {
 public:
  PixelFormat() = default;
  constexpr PixelFormat(uint8_t unique_id, PixelPacking packing_format,
                        uint8_t packed_bytes_per_pixel,
                        ComponentFormat component_format,
                        uint8_t component_bits_r, uint8_t component_bits_g,
                        uint8_t component_bits_b, uint8_t component_bits_a)
      : unique_id_(unique_id),
        packing_format_(packing_format),
        packed_bytes_per_pixel_(packed_bytes_per_pixel),
        component_format_(component_format),
        component_bits_r_(component_bits_r),
        component_bits_g_(component_bits_g),
        component_bits_b_(component_bits_b),
        component_bits_a_(component_bits_a) {}

  bool operator==(const PixelFormat& other) const {
    return unique_id_ == other.unique_id_;
  }
  bool operator!=(const PixelFormat& other) const { return !(*this == other); }

  // Returns the unique ID of the pixel format.
  // This byte value can be used in lookup tables as no other color format in
  // the system will have it.
  // Values of 0 always indicate 'undefined' formats.
  constexpr int unique_id() const { return unique_id_; }
  constexpr operator int() const { return unique_id_; }

  // Returns the packing mode of the pixels.
  constexpr PixelPacking packing_format() const { return packing_format_; }

  // Returns the format components are stored in.
  constexpr ComponentFormat component_format() const {
    return component_format_;
  }

  // Returns the total number of components in the pixel format.
  // For example, R8G8B8 would return 3.
  constexpr int component_count() const {
    return ((component_bits_r_ != 0) ? 1 : 0) +
           ((component_bits_g_ != 0) ? 1 : 0) +
           ((component_bits_b_ != 0) ? 1 : 0) +
           ((component_bits_a_ != 0) ? 1 : 0);
  }

  // Returns the size, in bits, of each component of the format.
  // A size of 0 indicates the component is not present in the format.
  constexpr int component_bits_r() const { return component_bits_r_; }
  constexpr int component_bits_g() const { return component_bits_g_; }
  constexpr int component_bits_b() const { return component_bits_b_; }
  constexpr int component_bits_a() const { return component_bits_a_; }
  constexpr int component_bits_depth() const { return component_bits_r_; }
  constexpr int component_bits_stencil() const { return component_bits_g_; }

  // Returns true if the given color format is a flexible type, meaning it can
  // be represented by one or more other types depending on system preference.
  // These formats are never used directly as a storage type but instead used to
  // query preferred types by various implementations.
  constexpr bool is_flexible() const {
    return (static_cast<uint8_t>(packing_format_) &
            static_cast<uint8_t>(PixelPacking::kFlexibleMask)) != 0;
  }

  // Returns true if the given format contains compressed data.
  constexpr bool is_compressed() const {
    return (static_cast<uint8_t>(packing_format_) &
            static_cast<uint8_t>(PixelPacking::kCompressedMask)) != 0;
  }

  // Returns true if the given format represents depth/stencil data.
  constexpr bool is_depth_stencil() const {
    return packing_format_ == PixelPacking::kDepthStencil;
  }

  // Returns true if the given color format includes an alpha channel or other
  // transparency information.
  constexpr bool has_transparency() const { return component_bits_a_ != 0; }

  // Returns true if the given color format is in linear gamma space.
  // Otherwise, the color format is gamma corrected (such as sRGB).
  constexpr bool is_linear() const {
    return component_format_ != ComponentFormat::kSrgb;
  }

  // Computes the total bytes required to store an image in this format.
  size_t ComputeDataSize(int width, int height) const {
    if (packed_bytes_per_pixel_) {
      return packed_bytes_per_pixel_ * width * height;
    } else {
      return ComputeCompressedDataSize(width, height);
    }
  }

 private:
  size_t ComputeCompressedDataSize(int width, int height) const;

  uint8_t unique_id_;
  PixelPacking packing_format_;
  uint8_t packed_bytes_per_pixel_;
  ComponentFormat component_format_;
  uint8_t component_bits_r_;
  uint8_t component_bits_g_;
  uint8_t component_bits_b_;
  uint8_t component_bits_a_;
};
static_assert(sizeof(PixelFormat) == sizeof(uint64_t),
              "Pixel format must be representable as a uint64_t");

// A simple lookup table with pixel formats as the keys.
// Use this to build dense compile-time-safe(ish) tables.
// In most cases this is just as efficient as a direct table but also provides
// safety to ensure there are no bad accesses.
//
// Usage:
//  struct MyEntry { GLint some_mapped_value; };
//  PixelFormatTable<MyEntry, PixelFormats::kEtc2R8G8B8UNorm,
//                            PixelFormats::kEtc2R8G8B8A8Srgb> table({
//      {GL_ETC2_A},  // kEtc2R8G8B8UNorm (first)
//      {GL_ETC2_B},  // kEtc2R8G8B8Srgb
//      {GL_ETC2_C},  // kEtc2R8G8B8A8Srgb (last)
//  });
//  MyEntry entry = table.Find(some_pixel_format);
template <typename T, int kFirstId, int kLastId, int kStride = 1>
class PixelFormatTable {
 private:
  static_assert(kFirstId <= kLastId, "Wrong order");
  static constexpr int kTableRange = kLastId - kFirstId + 1;
  static constexpr int kTableSize = kTableRange / kStride;

 public:
  explicit PixelFormatTable(T const (&table)[kTableSize]) : table_(table) {}

  // Returns the total range mapped by the table (size * stride).
  constexpr int range() const { return kTableRange; }
  // Returns the total size of the table in entries.
  constexpr int size() const { return kTableSize; }

  // Finds an entry in the table for the given pixel format.
  // DCHECKs if the value is out of range.
  const T& Find(uint8_t pixel_format_id) const {
    DCHECK(pixel_format_id >= kFirstId && pixel_format_id <= kLastId);
    if (pixel_format_id < kFirstId || pixel_format_id > kLastId) {
      return table_[0];  // Prevent bad access.
    }
    return table_[(pixel_format_id - kFirstId) / kStride];
  }
  const T& Find(PixelFormat pixel_format) const {
    return Find(pixel_format.unique_id());
  }

 private:
  T const (&table_)[kTableSize];
};

// Pixel formats that are supported throughout the system.
// Some of these are only available for use in the graphics system on certain
// platforms. Validate formats before using, or stick to commonly supported
// formats for safety.
//
// Vulkan formats:
// https://vulkan.lunarg.com/doc/view/1.0.30.0/linux/vkspec.chunked/ch31s03.html
//
// Metal formats:
// https://developer.apple.com/reference/metal/mtlpixelformat
//
// D3D formats:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
namespace PixelFormats {

// clang-format off

//                                    component_bits_a ---------------------------------------------------------------------------+
//                                    component_bits_b -----------------------------------------------------------------------+   |
//                                    component_bits_g -------------------------------------------------------------------+   |   |
//                                    component_bits_r ---------------------------------------------------------------+   |   |   |
//                                    component_format ------------------------------------+                          |   |   |   |
//                              packed_bytes_per_pixel ---------------------------------+  |                          |   |   |   |
//                                      packing_format ---+                             |  |                          |   |   |   |
//                                           unique_id +  |                             |  |                          |   |   |   |
//                                                     |  |                             |  |                          |   |   |   |
//                                                     v  v                             v  v                          v   v   v   v
constexpr static PixelFormat kUndefined           = {  0, PixelPacking::kOpaque,        0, ComponentFormat::kSFloat,  0,  0,  0,  0};

constexpr static PixelFormat kR4G4UNorm           = {  1, PixelPacking::kUncompressed,  1, ComponentFormat::kUNorm,   4,  4,  0,  0};
constexpr static PixelFormat kR4G4B4A4UNorm       = {  2, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   4,  4,  4,  4};
constexpr static PixelFormat kB4G4R4A4UNorm       = {  3, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   4,  4,  4,  4};
constexpr static PixelFormat kR5G6B5UNorm         = {  4, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  6,  5,  0};
constexpr static PixelFormat kB5G6R5UNorm         = {  5, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  6,  5,  0};
constexpr static PixelFormat kR5G5B5A1UNorm       = {  6, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  5,  5,  1};
constexpr static PixelFormat kB5G5R5A1UNorm       = {  7, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  5,  5,  1};
constexpr static PixelFormat kA1R5G5B5UNorm       = {  8, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  5,  5,  1};

constexpr static PixelFormat kR8UNorm             = {  9, PixelPacking::kUncompressed,  1, ComponentFormat::kUNorm,   8,  0,  0,  0};
constexpr static PixelFormat kR8SNorm             = { 10, PixelPacking::kUncompressed,  1, ComponentFormat::kSNorm,   8,  0,  0,  0};
constexpr static PixelFormat kR8UScaled           = { 11, PixelPacking::kUncompressed,  1, ComponentFormat::kUFloat,  8,  0,  0,  0};
constexpr static PixelFormat kR8SScaled           = { 12, PixelPacking::kUncompressed,  1, ComponentFormat::kSFloat,  8,  0,  0,  0};
constexpr static PixelFormat kR8UInt              = { 13, PixelPacking::kUncompressed,  1, ComponentFormat::kUInt,    8,  0,  0,  0};
constexpr static PixelFormat kR8SInt              = { 14, PixelPacking::kUncompressed,  1, ComponentFormat::kSInt,    8,  0,  0,  0};
constexpr static PixelFormat kR8Srgb              = { 15, PixelPacking::kUncompressed,  1, ComponentFormat::kSrgb,    8,  0,  0,  0};

constexpr static PixelFormat kR8G8UNorm           = { 16, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   8,  8,  0,  0};
constexpr static PixelFormat kR8G8SNorm           = { 17, PixelPacking::kUncompressed,  2, ComponentFormat::kSNorm,   8,  8,  0,  0};
constexpr static PixelFormat kR8G8UScaled         = { 18, PixelPacking::kUncompressed,  2, ComponentFormat::kUFloat,  8,  8,  0,  0};
constexpr static PixelFormat kR8G8SScaled         = { 19, PixelPacking::kUncompressed,  2, ComponentFormat::kSFloat,  8,  8,  0,  0};
constexpr static PixelFormat kR8G8UInt            = { 20, PixelPacking::kUncompressed,  2, ComponentFormat::kUInt,    8,  8,  0,  0};
constexpr static PixelFormat kR8G8SInt            = { 21, PixelPacking::kUncompressed,  2, ComponentFormat::kSInt,    8,  8,  0,  0};
constexpr static PixelFormat kR8G8Srgb            = { 22, PixelPacking::kUncompressed,  2, ComponentFormat::kSrgb,    8,  8,  0,  0};

constexpr static PixelFormat kR8G8B8UNorm         = { 23, PixelPacking::kUncompressed,  3, ComponentFormat::kUNorm,   8,  8,  8,  0};
constexpr static PixelFormat kR8G8B8SNorm         = { 24, PixelPacking::kUncompressed,  3, ComponentFormat::kSNorm,   8,  8,  8,  0};
constexpr static PixelFormat kR8G8B8UScaled       = { 25, PixelPacking::kUncompressed,  3, ComponentFormat::kUFloat,  8,  8,  8,  0};
constexpr static PixelFormat kR8G8B8SScaled       = { 26, PixelPacking::kUncompressed,  3, ComponentFormat::kSFloat,  8,  8,  8,  0};
constexpr static PixelFormat kR8G8B8UInt          = { 27, PixelPacking::kUncompressed,  3, ComponentFormat::kUInt,    8,  8,  8,  0};
constexpr static PixelFormat kR8G8B8SInt          = { 28, PixelPacking::kUncompressed,  3, ComponentFormat::kSInt,    8,  8,  8,  0};
constexpr static PixelFormat kR8G8B8Srgb          = { 29, PixelPacking::kUncompressed,  3, ComponentFormat::kSrgb,    8,  8,  8,  0};

constexpr static PixelFormat kB8G8R8UNorm         = { 30, PixelPacking::kUncompressed,  3, ComponentFormat::kUNorm,   8,  8,  8,  0};
constexpr static PixelFormat kB8G8R8SNorm         = { 31, PixelPacking::kUncompressed,  3, ComponentFormat::kSNorm,   8,  8,  8,  0};
constexpr static PixelFormat kB8G8R8UScaled       = { 32, PixelPacking::kUncompressed,  3, ComponentFormat::kUFloat,  8,  8,  8,  0};
constexpr static PixelFormat kB8G8R8SScaled       = { 33, PixelPacking::kUncompressed,  3, ComponentFormat::kSFloat,  8,  8,  8,  0};
constexpr static PixelFormat kB8G8R8UInt          = { 34, PixelPacking::kUncompressed,  3, ComponentFormat::kUInt,    8,  8,  8,  0};
constexpr static PixelFormat kB8G8R8SInt          = { 35, PixelPacking::kUncompressed,  3, ComponentFormat::kSInt,    8,  8,  8,  0};
constexpr static PixelFormat kB8G8R8Srgb          = { 36, PixelPacking::kUncompressed,  3, ComponentFormat::kSrgb,    8,  8,  8,  0};

constexpr static PixelFormat kR8G8B8A8UNorm       = { 37, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8SNorm       = { 38, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,   8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8UScaled     = { 39, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat,  8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8SScaled     = { 40, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat,  8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8UInt        = { 41, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,    8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8SInt        = { 42, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,    8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8Srgb        = { 43, PixelPacking::kUncompressed,  4, ComponentFormat::kSrgb,    8,  8,  8,  8};

constexpr static PixelFormat kB8G8R8A8UNorm       = { 44, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8SNorm       = { 45, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,   8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8UScaled     = { 46, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat,  8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8SScaled     = { 47, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat,  8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8UInt        = { 48, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,    8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8SInt        = { 49, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,    8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8Srgb        = { 50, PixelPacking::kUncompressed,  4, ComponentFormat::kSrgb,    8,  8,  8,  8};

constexpr static PixelFormat kA8B8G8R8UNorm       = { 51, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kA8B8G8R8SNorm       = { 52, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,   8,  8,  8,  8};
constexpr static PixelFormat kA8B8G8R8UScaled     = { 53, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat,  8,  8,  8,  8};
constexpr static PixelFormat kA8B8G8R8SScaled     = { 54, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat,  8,  8,  8,  8};
constexpr static PixelFormat kA8B8G8R8UInt        = { 55, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,    8,  8,  8,  8};
constexpr static PixelFormat kA8B8G8R8SInt        = { 56, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,    8,  8,  8,  8};
constexpr static PixelFormat kA8B8G8R8Srgb        = { 57, PixelPacking::kUncompressed,  4, ComponentFormat::kSrgb,    8,  8,  8,  8};

constexpr static PixelFormat kA2R10G10B10UNorm    = { 58, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,  10, 10, 10,  2};
constexpr static PixelFormat kA2R10G10B10SNorm    = { 59, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,  10, 10, 10,  2};
constexpr static PixelFormat kA2R10G10B10UScaled  = { 60, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat, 10, 10, 10,  2};
constexpr static PixelFormat kA2R10G10B10SScaled  = { 61, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 10, 10, 10,  2};
constexpr static PixelFormat kA2R10G10B10UInt     = { 62, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   10, 10, 10,  2};
constexpr static PixelFormat kA2R10G10B10SInt     = { 63, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   10, 10, 10,  2};

constexpr static PixelFormat kA2B10G10R10UNorm    = { 64, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,  10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10SNorm    = { 65, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,  10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10UScaled  = { 66, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat, 10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10SScaled  = { 67, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10UInt     = { 68, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10SInt     = { 69, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   10, 10, 10,  2};

constexpr static PixelFormat kR16UNorm            = { 70, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,  16,  0,  0,  0};
constexpr static PixelFormat kR16SNorm            = { 71, PixelPacking::kUncompressed,  2, ComponentFormat::kSNorm,  16,  0,  0,  0};
constexpr static PixelFormat kR16UScaled          = { 72, PixelPacking::kUncompressed,  2, ComponentFormat::kUFloat, 16,  0,  0,  0};
constexpr static PixelFormat kR16SScaled          = { 73, PixelPacking::kUncompressed,  2, ComponentFormat::kSFloat, 16,  0,  0,  0};
constexpr static PixelFormat kR16UInt             = { 74, PixelPacking::kUncompressed,  2, ComponentFormat::kUInt,   16,  0,  0,  0};
constexpr static PixelFormat kR16SInt             = { 75, PixelPacking::kUncompressed,  2, ComponentFormat::kSInt,   16,  0,  0,  0};
constexpr static PixelFormat kR16SFloat           = { 76, PixelPacking::kUncompressed,  2, ComponentFormat::kSFloat, 16,  0,  0,  0};

constexpr static PixelFormat kR16G16UNorm         = { 77, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,  16, 16,  0,  0};
constexpr static PixelFormat kR16G16SNorm         = { 78, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,  16, 16,  0,  0};
constexpr static PixelFormat kR16G16UScaled       = { 79, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat, 16, 16,  0,  0};
constexpr static PixelFormat kR16G16SScaled       = { 80, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 16, 16,  0,  0};
constexpr static PixelFormat kR16G16UInt          = { 81, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   16, 16,  0,  0};
constexpr static PixelFormat kR16G16SInt          = { 82, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   16, 16,  0,  0};
constexpr static PixelFormat kR16G16SFloat        = { 83, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 16, 16,  0,  0};

constexpr static PixelFormat kR16G16B16UNorm      = { 84, PixelPacking::kUncompressed,  6, ComponentFormat::kUNorm,  16, 16, 16,  0};
constexpr static PixelFormat kR16G16B16SNorm      = { 85, PixelPacking::kUncompressed,  6, ComponentFormat::kSNorm,  16, 16, 16,  0};
constexpr static PixelFormat kR16G16B16UScaled    = { 86, PixelPacking::kUncompressed,  6, ComponentFormat::kUFloat, 16, 16, 16,  0};
constexpr static PixelFormat kR16G16B16SScaled    = { 87, PixelPacking::kUncompressed,  6, ComponentFormat::kSFloat, 16, 16, 16,  0};
constexpr static PixelFormat kR16G16B16UInt       = { 88, PixelPacking::kUncompressed,  6, ComponentFormat::kUInt,   16, 16, 16,  0};
constexpr static PixelFormat kR16G16B16SInt       = { 89, PixelPacking::kUncompressed,  6, ComponentFormat::kSInt,   16, 16, 16,  0};
constexpr static PixelFormat kR16G16B16SFloat     = { 90, PixelPacking::kUncompressed,  6, ComponentFormat::kSFloat, 16, 16, 16,  0};

constexpr static PixelFormat kR16G16B16A16UNorm   = { 91, PixelPacking::kUncompressed,  8, ComponentFormat::kUNorm,  16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SNorm   = { 92, PixelPacking::kUncompressed,  8, ComponentFormat::kSNorm,  16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16UScaled = { 93, PixelPacking::kUncompressed,  8, ComponentFormat::kUFloat, 16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SScaled = { 94, PixelPacking::kUncompressed,  8, ComponentFormat::kSFloat, 16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16UInt    = { 95, PixelPacking::kUncompressed,  8, ComponentFormat::kUInt,   16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SInt    = { 96, PixelPacking::kUncompressed,  8, ComponentFormat::kSInt,   16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SFloat  = { 97, PixelPacking::kUncompressed,  8, ComponentFormat::kSFloat, 16, 16, 16, 16};

constexpr static PixelFormat kR32UInt             = { 98, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   32,  0,  0,  0};
constexpr static PixelFormat kR32SInt             = { 99, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   32,  0,  0,  0};
constexpr static PixelFormat kR32SFloat           = {100, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 32,  0,  0,  0};

constexpr static PixelFormat kR32G32UInt          = {101, PixelPacking::kUncompressed,  8, ComponentFormat::kUInt,   32, 32,  0,  0};
constexpr static PixelFormat kR32G32SInt          = {102, PixelPacking::kUncompressed,  8, ComponentFormat::kSInt,   32, 32,  0,  0};
constexpr static PixelFormat kR32G32SFloat        = {103, PixelPacking::kUncompressed,  8, ComponentFormat::kSFloat, 32, 32,  0,  0};

constexpr static PixelFormat kR32G32B32UInt       = {104, PixelPacking::kUncompressed, 12, ComponentFormat::kUInt,   32, 32, 32,  0};
constexpr static PixelFormat kR32G32B32SInt       = {105, PixelPacking::kUncompressed, 12, ComponentFormat::kSInt,   32, 32, 32,  0};
constexpr static PixelFormat kR32G32B32SFloat     = {106, PixelPacking::kUncompressed, 12, ComponentFormat::kSFloat, 32, 32, 32,  0};

constexpr static PixelFormat kR32G32B32A32UInt    = {107, PixelPacking::kUncompressed, 16, ComponentFormat::kUInt,   32, 32, 32, 32};
constexpr static PixelFormat kR32G32B32A32SInt    = {108, PixelPacking::kUncompressed, 16, ComponentFormat::kSInt,   32, 32, 32, 32};
constexpr static PixelFormat kR32G32B32A32SFloat  = {109, PixelPacking::kUncompressed, 16, ComponentFormat::kSFloat, 32, 32, 32, 32};

constexpr static PixelFormat kR64UInt             = {110, PixelPacking::kUncompressed,  8, ComponentFormat::kUInt,   64,  0,  0,  0};
constexpr static PixelFormat kR64SInt             = {111, PixelPacking::kUncompressed,  8, ComponentFormat::kSInt,   64,  0,  0,  0};
constexpr static PixelFormat kR64SFloat           = {112, PixelPacking::kUncompressed,  8, ComponentFormat::kSFloat, 64,  0,  0,  0};

constexpr static PixelFormat kR64G64UInt          = {113, PixelPacking::kUncompressed, 16, ComponentFormat::kUInt,   64, 64,  0,  0};
constexpr static PixelFormat kR64G64SInt          = {114, PixelPacking::kUncompressed, 16, ComponentFormat::kSInt,   64, 64,  0,  0};
constexpr static PixelFormat kR64G64SFloat        = {115, PixelPacking::kUncompressed, 16, ComponentFormat::kSFloat, 64, 64,  0,  0};

constexpr static PixelFormat kR64G64B64UInt       = {116, PixelPacking::kUncompressed, 24, ComponentFormat::kUInt,   64, 64, 64,  0};
constexpr static PixelFormat kR64G64B64SInt       = {117, PixelPacking::kUncompressed, 24, ComponentFormat::kSInt,   64, 64, 64,  0};
constexpr static PixelFormat kR64G64B64SFloat     = {118, PixelPacking::kUncompressed, 24, ComponentFormat::kSFloat, 64, 64, 64,  0};

constexpr static PixelFormat kR64G64B64A64UInt    = {119, PixelPacking::kUncompressed, 32, ComponentFormat::kUInt,   64, 64, 64, 64};
constexpr static PixelFormat kR64G64B64A64SInt    = {120, PixelPacking::kUncompressed, 32, ComponentFormat::kSInt,   64, 64, 64, 64};
constexpr static PixelFormat kR64G64B64A64SFloat  = {121, PixelPacking::kUncompressed, 32, ComponentFormat::kSFloat, 64, 64, 64, 64};

constexpr static PixelFormat kB10G11R11UFloat     = {122, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat, 11, 11, 10,  0};
constexpr static PixelFormat kE5B9G9R9UFloat      = {123, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat,  9,  9,  9,  5};

constexpr static PixelFormat kD16UNorm            = {124, PixelPacking::kDepthStencil,  2, ComponentFormat::kUNorm,  16,  0,  0,  0};
constexpr static PixelFormat kX8D24UNorm          = {125, PixelPacking::kDepthStencil,  3, ComponentFormat::kUNorm,  24,  8,  0,  0};
constexpr static PixelFormat kD32SFloat           = {126, PixelPacking::kDepthStencil,  4, ComponentFormat::kSFloat, 32,  0,  0,  0};
constexpr static PixelFormat kS8UInt              = {127, PixelPacking::kDepthStencil,  1, ComponentFormat::kUInt,    0,  8,  0,  0};
constexpr static PixelFormat kD16UNormS8UInt      = {128, PixelPacking::kDepthStencil,  3, ComponentFormat::kUNorm,  16,  8,  0,  0};
constexpr static PixelFormat kD24UNormS8UInt      = {129, PixelPacking::kDepthStencil,  4, ComponentFormat::kUNorm,  24,  8,  0,  0};
constexpr static PixelFormat kD32SFloatS8UInt     = {130, PixelPacking::kDepthStencil,  4, ComponentFormat::kSFloat, 32,  8,  0,  0};

constexpr static PixelFormat kBC1RGBUNorm         = {131, PixelPacking::kBC1,           0, ComponentFormat::kUNorm,   1,  1,  1,  0};
constexpr static PixelFormat kBC1RGBSrgb          = {132, PixelPacking::kBC1,           0, ComponentFormat::kSrgb,    1,  1,  1,  0};
constexpr static PixelFormat kBC1RGBAUNorm        = {133, PixelPacking::kBC1,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC1RGBASrgb         = {134, PixelPacking::kBC1,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kBC2UNorm            = {135, PixelPacking::kBC2,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC2Srgb             = {136, PixelPacking::kBC2,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kBC3UNorm            = {137, PixelPacking::kBC3,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC3Srgb             = {138, PixelPacking::kBC3,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kBC4UNorm            = {139, PixelPacking::kBC4,           0, ComponentFormat::kUNorm,   1,  0,  0,  0};
constexpr static PixelFormat kBC4SNorm            = {140, PixelPacking::kBC4,           0, ComponentFormat::kSNorm,   1,  0,  0,  0};
constexpr static PixelFormat kBC5UNorm            = {141, PixelPacking::kBC5,           0, ComponentFormat::kUNorm,   1,  1,  0,  0};
constexpr static PixelFormat kBC5SNorm            = {142, PixelPacking::kBC5,           0, ComponentFormat::kSNorm,   1,  1,  0,  0};
constexpr static PixelFormat kBC6HUFloat          = {143, PixelPacking::kBC6,           0, ComponentFormat::kUFloat,  1,  1,  1,  0};
constexpr static PixelFormat kBC6HSFloat          = {144, PixelPacking::kBC6,           0, ComponentFormat::kSFloat,  1,  1,  1,  0};
constexpr static PixelFormat kBC7UNorm            = {145, PixelPacking::kBC7,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC7Srgb             = {146, PixelPacking::kBC7,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};

constexpr static PixelFormat kEtc2R8G8B8UNorm     = {147, PixelPacking::kEtc2,          0, ComponentFormat::kUNorm,   8,  8,  8,  0};
constexpr static PixelFormat kEtc2R8G8B8Srgb      = {148, PixelPacking::kEtc2,          0, ComponentFormat::kSrgb,    8,  8,  8,  0};
constexpr static PixelFormat kEtc2R8G8B8A1UNorm   = {149, PixelPacking::kEtc2,          0, ComponentFormat::kUNorm,   8,  8,  8,  1};
constexpr static PixelFormat kEtc2R8G8B8A1Srgb    = {150, PixelPacking::kEtc2,          0, ComponentFormat::kSrgb,    8,  8,  8,  1};
constexpr static PixelFormat kEtc2R8G8B8A8UNorm   = {151, PixelPacking::kEtc2,          0, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kEtc2R8G8B8A8Srgb    = {152, PixelPacking::kEtc2,          0, ComponentFormat::kSrgb,    8,  8,  8,  8};

constexpr static PixelFormat kEacR11UNorm         = {153, PixelPacking::kEac,           0, ComponentFormat::kUNorm,  11,  0,  0,  0};
constexpr static PixelFormat kEacR11SNorm         = {154, PixelPacking::kEac,           0, ComponentFormat::kSNorm,  11,  0,  0,  0};
constexpr static PixelFormat kEacR11G11UNorm      = {155, PixelPacking::kEac,           0, ComponentFormat::kUNorm,  11, 11,  0,  0};
constexpr static PixelFormat kEacR11G11SNorm      = {156, PixelPacking::kEac,           0, ComponentFormat::kSNorm,  11, 11,  0,  0};

constexpr static PixelFormat kAstc4x4UNorm        = {157, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc4x4Srgb         = {158, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc5x4UNorm        = {159, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc5x4Srgb         = {160, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc5x5UNorm        = {161, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc5x5Srgb         = {162, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc6x5UNorm        = {163, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc6x5Srgb         = {164, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc6x6UNorm        = {165, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc6x6Srgb         = {166, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc8x5UNorm        = {167, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc8x5Srgb         = {168, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc8x6UNorm        = {169, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc8x6Srgb         = {170, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc8x8UNorm        = {171, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc8x8Srgb         = {172, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x5UNorm       = {173, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x5Srgb        = {174, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x6UNorm       = {175, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x6Srgb        = {176, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x8UNorm       = {177, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x8Srgb        = {178, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x10UNorm      = {179, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x10Srgb       = {180, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc12x10UNorm      = {181, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc12x10Srgb       = {182, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc12x12UNorm      = {183, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc12x12Srgb       = {184, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};

// TODO(benvanik): PVRTC formats.

// Something missing from this list? Append only! IDs must be dense and you'll
// likely get some compile warnings about tables that need updating.
//
// Instructions:
// - Append a new PixelFormat to the end of the table above with a new ID.
// - Match the formatting; lint won't do it for you.
// - Compile with --keep_going and wait for all the errors (maybe none?!).

// clang-format on

}  // namespace PixelFormats

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_PIXEL_FORMAT_H_
