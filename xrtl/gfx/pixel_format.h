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
//  static const MyEntry kTable[] = {
//      {GL_ETC2_A},  // kEtc2R8G8B8UNorm (first)
//      {GL_ETC2_B},  // kEtc2R8G8B8Srgb
//      {GL_ETC2_C},  // kEtc2R8G8B8A8Srgb (last)
//  };
//  PixelFormatTable<MyEntry, PixelFormats::kEtc2R8G8B8UNorm,
//                            PixelFormats::kEtc2R8G8B8A8Srgb> table(kTable);
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

constexpr static PixelFormat kR4G4B4A4UNorm       = {  1, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   4,  4,  4,  4};
constexpr static PixelFormat kR5G6B5UNorm         = {  2, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  6,  5,  0};
constexpr static PixelFormat kR5G5B5A1UNorm       = {  3, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   5,  5,  5,  1};

constexpr static PixelFormat kR8UNorm             = {  4, PixelPacking::kUncompressed,  1, ComponentFormat::kUNorm,   8,  0,  0,  0};
constexpr static PixelFormat kR8SNorm             = {  5, PixelPacking::kUncompressed,  1, ComponentFormat::kSNorm,   8,  0,  0,  0};
constexpr static PixelFormat kR8UInt              = {  6, PixelPacking::kUncompressed,  1, ComponentFormat::kUInt,    8,  0,  0,  0};
constexpr static PixelFormat kR8SInt              = {  7, PixelPacking::kUncompressed,  1, ComponentFormat::kSInt,    8,  0,  0,  0};

constexpr static PixelFormat kR8G8UNorm           = {  8, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,   8,  8,  0,  0};
constexpr static PixelFormat kR8G8SNorm           = {  9, PixelPacking::kUncompressed,  2, ComponentFormat::kSNorm,   8,  8,  0,  0};
constexpr static PixelFormat kR8G8UInt            = { 10, PixelPacking::kUncompressed,  2, ComponentFormat::kUInt,    8,  8,  0,  0};
constexpr static PixelFormat kR8G8SInt            = { 11, PixelPacking::kUncompressed,  2, ComponentFormat::kSInt,    8,  8,  0,  0};

constexpr static PixelFormat kR8G8B8A8UNorm       = { 12, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8SNorm       = { 13, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,   8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8UInt        = { 14, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,    8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8SInt        = { 15, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,    8,  8,  8,  8};
constexpr static PixelFormat kR8G8B8A8Srgb        = { 16, PixelPacking::kUncompressed,  4, ComponentFormat::kSrgb,    8,  8,  8,  8};

constexpr static PixelFormat kB8G8R8A8UNorm       = { 17, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kB8G8R8A8Srgb        = { 18, PixelPacking::kUncompressed,  4, ComponentFormat::kSrgb,    8,  8,  8,  8};

constexpr static PixelFormat kA2B10G10R10UNorm    = { 19, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,  10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10SNorm    = { 20, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,  10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10UInt     = { 21, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   10, 10, 10,  2};
constexpr static PixelFormat kA2B10G10R10SInt     = { 22, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   10, 10, 10,  2};

constexpr static PixelFormat kR16UNorm            = { 23, PixelPacking::kUncompressed,  2, ComponentFormat::kUNorm,  16,  0,  0,  0};
constexpr static PixelFormat kR16SNorm            = { 24, PixelPacking::kUncompressed,  2, ComponentFormat::kSNorm,  16,  0,  0,  0};
constexpr static PixelFormat kR16UInt             = { 25, PixelPacking::kUncompressed,  2, ComponentFormat::kUInt,   16,  0,  0,  0};
constexpr static PixelFormat kR16SInt             = { 26, PixelPacking::kUncompressed,  2, ComponentFormat::kSInt,   16,  0,  0,  0};
constexpr static PixelFormat kR16SFloat           = { 27, PixelPacking::kUncompressed,  2, ComponentFormat::kSFloat, 16,  0,  0,  0};

constexpr static PixelFormat kR16G16UNorm         = { 28, PixelPacking::kUncompressed,  4, ComponentFormat::kUNorm,  16, 16,  0,  0};
constexpr static PixelFormat kR16G16SNorm         = { 29, PixelPacking::kUncompressed,  4, ComponentFormat::kSNorm,  16, 16,  0,  0};
constexpr static PixelFormat kR16G16UInt          = { 30, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   16, 16,  0,  0};
constexpr static PixelFormat kR16G16SInt          = { 31, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   16, 16,  0,  0};
constexpr static PixelFormat kR16G16SFloat        = { 32, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 16, 16,  0,  0};

constexpr static PixelFormat kR16G16B16A16UNorm   = { 33, PixelPacking::kUncompressed,  8, ComponentFormat::kUNorm,  16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SNorm   = { 34, PixelPacking::kUncompressed,  8, ComponentFormat::kSNorm,  16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16UInt    = { 35, PixelPacking::kUncompressed,  8, ComponentFormat::kUInt,   16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SInt    = { 36, PixelPacking::kUncompressed,  8, ComponentFormat::kSInt,   16, 16, 16, 16};
constexpr static PixelFormat kR16G16B16A16SFloat  = { 37, PixelPacking::kUncompressed,  8, ComponentFormat::kSFloat, 16, 16, 16, 16};

constexpr static PixelFormat kR32UInt             = { 38, PixelPacking::kUncompressed,  4, ComponentFormat::kUInt,   32,  0,  0,  0};
constexpr static PixelFormat kR32SInt             = { 39, PixelPacking::kUncompressed,  4, ComponentFormat::kSInt,   32,  0,  0,  0};
constexpr static PixelFormat kR32SFloat           = { 40, PixelPacking::kUncompressed,  4, ComponentFormat::kSFloat, 32,  0,  0,  0};

constexpr static PixelFormat kR32G32UInt          = { 41, PixelPacking::kUncompressed,  8, ComponentFormat::kUInt,   32, 32,  0,  0};
constexpr static PixelFormat kR32G32SInt          = { 42, PixelPacking::kUncompressed,  8, ComponentFormat::kSInt,   32, 32,  0,  0};
constexpr static PixelFormat kR32G32SFloat        = { 43, PixelPacking::kUncompressed,  8, ComponentFormat::kSFloat, 32, 32,  0,  0};

constexpr static PixelFormat kR32G32B32UInt       = { 44, PixelPacking::kUncompressed, 12, ComponentFormat::kUInt,   32, 32, 32,  0};
constexpr static PixelFormat kR32G32B32SInt       = { 45, PixelPacking::kUncompressed, 12, ComponentFormat::kSInt,   32, 32, 32,  0};
constexpr static PixelFormat kR32G32B32SFloat     = { 46, PixelPacking::kUncompressed, 12, ComponentFormat::kSFloat, 32, 32, 32,  0};

constexpr static PixelFormat kR32G32B32A32UInt    = { 47, PixelPacking::kUncompressed, 16, ComponentFormat::kUInt,   32, 32, 32, 32};
constexpr static PixelFormat kR32G32B32A32SInt    = { 48, PixelPacking::kUncompressed, 16, ComponentFormat::kSInt,   32, 32, 32, 32};
constexpr static PixelFormat kR32G32B32A32SFloat  = { 49, PixelPacking::kUncompressed, 16, ComponentFormat::kSFloat, 32, 32, 32, 32};

constexpr static PixelFormat kB10G11R11UFloat     = { 50, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat, 11, 11, 10,  0};
constexpr static PixelFormat kE5B9G9R9UFloat      = { 51, PixelPacking::kUncompressed,  4, ComponentFormat::kUFloat,  9,  9,  9,  5};

constexpr static PixelFormat kD32SFloat           = { 52, PixelPacking::kDepthStencil,  4, ComponentFormat::kSFloat, 32,  0,  0,  0};
// Requires Device::Features pixel_formats.packed_depth_stencil.
constexpr static PixelFormat kD24UNormS8UInt      = { 53, PixelPacking::kDepthStencil,  4, ComponentFormat::kUNorm,  24,  8,  0,  0};
constexpr static PixelFormat kD32SFloatS8UInt     = { 54, PixelPacking::kDepthStencil,  4, ComponentFormat::kSFloat, 32,  8,  0,  0};

// Requires Device::Features pixel_formats.bc1_2_3.
constexpr static PixelFormat kBC1RGBAUNorm        = { 55, PixelPacking::kBC1,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC1RGBASrgb         = { 56, PixelPacking::kBC1,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kBC2UNorm            = { 57, PixelPacking::kBC2,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC2Srgb             = { 58, PixelPacking::kBC2,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kBC3UNorm            = { 59, PixelPacking::kBC3,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC3Srgb             = { 60, PixelPacking::kBC3,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};

// Requires Device::Features pixel_formats.bc4_5_6_7.
constexpr static PixelFormat kBC4UNorm            = { 61, PixelPacking::kBC4,           0, ComponentFormat::kUNorm,   1,  0,  0,  0};
constexpr static PixelFormat kBC4SNorm            = { 62, PixelPacking::kBC4,           0, ComponentFormat::kSNorm,   1,  0,  0,  0};
constexpr static PixelFormat kBC5UNorm            = { 63, PixelPacking::kBC5,           0, ComponentFormat::kUNorm,   1,  1,  0,  0};
constexpr static PixelFormat kBC5SNorm            = { 64, PixelPacking::kBC5,           0, ComponentFormat::kSNorm,   1,  1,  0,  0};
constexpr static PixelFormat kBC6HUFloat          = { 65, PixelPacking::kBC6,           0, ComponentFormat::kUFloat,  1,  1,  1,  0};
constexpr static PixelFormat kBC6HSFloat          = { 66, PixelPacking::kBC6,           0, ComponentFormat::kSFloat,  1,  1,  1,  0};
constexpr static PixelFormat kBC7UNorm            = { 67, PixelPacking::kBC7,           0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kBC7Srgb             = { 68, PixelPacking::kBC7,           0, ComponentFormat::kSrgb,    1,  1,  1,  1};

// Requires Device::Features pixel_formats.etc2.
constexpr static PixelFormat kEtc2R8G8B8UNorm     = { 69, PixelPacking::kEtc2,          0, ComponentFormat::kUNorm,   8,  8,  8,  0};
constexpr static PixelFormat kEtc2R8G8B8Srgb      = { 70, PixelPacking::kEtc2,          0, ComponentFormat::kSrgb,    8,  8,  8,  0};
constexpr static PixelFormat kEtc2R8G8B8A1UNorm   = { 71, PixelPacking::kEtc2,          0, ComponentFormat::kUNorm,   8,  8,  8,  1};
constexpr static PixelFormat kEtc2R8G8B8A1Srgb    = { 72, PixelPacking::kEtc2,          0, ComponentFormat::kSrgb,    8,  8,  8,  1};
constexpr static PixelFormat kEtc2R8G8B8A8UNorm   = { 73, PixelPacking::kEtc2,          0, ComponentFormat::kUNorm,   8,  8,  8,  8};
constexpr static PixelFormat kEtc2R8G8B8A8Srgb    = { 74, PixelPacking::kEtc2,          0, ComponentFormat::kSrgb,    8,  8,  8,  8};

// Requires Device::Features pixel_formats.eac.
constexpr static PixelFormat kEacR11UNorm         = { 75, PixelPacking::kEac,           0, ComponentFormat::kUNorm,  11,  0,  0,  0};
constexpr static PixelFormat kEacR11SNorm         = { 76, PixelPacking::kEac,           0, ComponentFormat::kSNorm,  11,  0,  0,  0};
constexpr static PixelFormat kEacR11G11UNorm      = { 77, PixelPacking::kEac,           0, ComponentFormat::kUNorm,  11, 11,  0,  0};
constexpr static PixelFormat kEacR11G11SNorm      = { 78, PixelPacking::kEac,           0, ComponentFormat::kSNorm,  11, 11,  0,  0};

// Requires Device::Features pixel_formats.astc.
constexpr static PixelFormat kAstc4x4UNorm        = { 79, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc4x4Srgb         = { 80, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc5x4UNorm        = { 81, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc5x4Srgb         = { 82, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc5x5UNorm        = { 83, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc5x5Srgb         = { 84, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc6x5UNorm        = { 85, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc6x5Srgb         = { 86, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc6x6UNorm        = { 87, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc6x6Srgb         = { 88, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc8x5UNorm        = { 89, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc8x5Srgb         = { 90, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc8x6UNorm        = { 91, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc8x6Srgb         = { 92, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc8x8UNorm        = { 93, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc8x8Srgb         = { 94, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x5UNorm       = { 95, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x5Srgb        = { 96, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x6UNorm       = { 97, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x6Srgb        = { 98, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x8UNorm       = { 99, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x8Srgb        = {100, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc10x10UNorm      = {101, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc10x10Srgb       = {102, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc12x10UNorm      = {103, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc12x10Srgb       = {104, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};
constexpr static PixelFormat kAstc12x12UNorm      = {105, PixelPacking::kAstc,          0, ComponentFormat::kUNorm,   1,  1,  1,  1};
constexpr static PixelFormat kAstc12x12Srgb       = {106, PixelPacking::kAstc,          0, ComponentFormat::kSrgb,    1,  1,  1,  1};

// Requires Device::Features pixel_formats.pvrtc.
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
