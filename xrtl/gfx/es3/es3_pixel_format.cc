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

#include "xrtl/gfx/es3/es3_pixel_format.h"

namespace xrtl {
namespace gfx {
namespace es3 {

GLenum ConvertPixelFormatToInternalFormat(PixelFormat pixel_format) {
  static const PixelFormatTable<GLenum, PixelFormats::kUndefined,
                                PixelFormats::kAstc12x12Srgb>
      kInternalFormats({
          GL_NONE,                                       // kUndefined
                                                         //
          GL_NONE,                                       // kR4G4UNorm
          GL_RGBA4,                                      // kR4G4B4A4UNorm
          GL_RGBA4,                                      // kB4G4R4A4UNorm
          GL_RGB565,                                     // kR5G6B5UNorm
          GL_RGB565,                                     // kB5G6R5UNorm
          GL_RGB5_A1,                                    // kR5G5B5A1UNorm
          GL_RGB5_A1,                                    // kB5G5R5A1UNorm
          GL_RGB5_A1,                                    // kA1R5G5B5UNorm
                                                         //
          GL_R8,                                         // kR8UNorm
          GL_R8_SNORM,                                   // kR8SNorm
          GL_R8,                                         // kR8UScaled
          GL_R8_SNORM,                                   // kR8SScaled
          GL_R8UI,                                       // kR8UInt
          GL_R8I,                                        // kR8SInt
          GL_R8,                                         // kR8Srgb
                                                         //
          GL_RG8,                                        // kR8G8UNorm
          GL_RG8_SNORM,                                  // kR8G8SNorm
          GL_RG8,                                        // kR8G8UScaled
          GL_RG8_SNORM,                                  // kR8G8SScaled
          GL_RG8UI,                                      // kR8G8UInt
          GL_RG8I,                                       // kR8G8SInt
          GL_RG8,                                        // kR8G8Srgb
                                                         //
          GL_RGB8,                                       // kR8G8B8UNorm
          GL_RGB8_SNORM,                                 // kR8G8B8SNorm
          GL_RGB8,                                       // kR8G8B8UScaled
          GL_RGB8_SNORM,                                 // kR8G8B8SScaled
          GL_RGB8UI,                                     // kR8G8B8UInt
          GL_RGB8I,                                      // kR8G8B8SInt
          GL_SRGB8,                                      // kR8G8B8Srgb
                                                         //
          GL_RGB8,                                       // kB8G8R8UNorm
          GL_RGB8_SNORM,                                 // kB8G8R8SNorm
          GL_RGB8,                                       // kB8G8R8UScaled
          GL_RGB8_SNORM,                                 // kB8G8R8SScaled
          GL_RGB8UI,                                     // kB8G8R8UInt
          GL_RGB8I,                                      // kB8G8R8SInt
          GL_SRGB8,                                      // kB8G8R8Srgb
                                                         //
          GL_RGBA8,                                      // kR8G8B8A8UNorm
          GL_RGBA8_SNORM,                                // kR8G8B8A8SNorm
          GL_RGBA8,                                      // kR8G8B8A8UScaled
          GL_RGBA8_SNORM,                                // kR8G8B8A8SScaled
          GL_RGBA8UI,                                    // kR8G8B8A8UInt
          GL_RGBA8I,                                     // kR8G8B8A8SInt
          GL_SRGB8_ALPHA8,                               // kR8G8B8A8Srgb
                                                         //
          GL_RGBA8,                                      // kB8G8R8A8UNorm
          GL_RGBA8_SNORM,                                // kB8G8R8A8SNorm
          GL_RGBA8,                                      // kB8G8R8A8UScaled
          GL_RGBA8_SNORM,                                // kB8G8R8A8SScaled
          GL_RGBA8UI,                                    // kB8G8R8A8UInt
          GL_RGBA8I,                                     // kB8G8R8A8SInt
          GL_SRGB8_ALPHA8,                               // kB8G8R8A8Srgb
                                                         //
          GL_RGBA8,                                      // kA8B8G8R8UNorm
          GL_RGBA8_SNORM,                                // kA8B8G8R8SNorm
          GL_RGBA8,                                      // kA8B8G8R8UScaled
          GL_RGBA8_SNORM,                                // kA8B8G8R8SScaled
          GL_RGBA8UI,                                    // kA8B8G8R8UInt
          GL_RGBA8I,                                     // kA8B8G8R8SInt
          GL_SRGB8_ALPHA8,                               // kA8B8G8R8Srgb
                                                         //
          GL_RGB10_A2,                                   // kA2R10G10B10UNorm
          GL_RGB10_A2,                                   // kA2R10G10B10SNorm
          GL_RGB10_A2,                                   // kA2R10G10B10UScaled
          GL_RGB10_A2,                                   // kA2R10G10B10SScaled
          GL_RGB10_A2UI,                                 // kA2R10G10B10UInt
          GL_RGB10_A2UI,                                 // kA2R10G10B10SInt
                                                         //
          GL_RGB10_A2,                                   // kA2B10G10R10UNorm
          GL_RGB10_A2,                                   // kA2B10G10R10SNorm
          GL_RGB10_A2,                                   // kA2B10G10R10UScaled
          GL_RGB10_A2,                                   // kA2B10G10R10SScaled
          GL_RGB10_A2UI,                                 // kA2B10G10R10UInt
          GL_RGB10_A2UI,                                 // kA2B10G10R10SInt
                                                         //
          GL_R16F,                                       // kR16UNorm
          GL_R16F,                                       // kR16SNorm
          GL_R16F,                                       // kR16UScaled
          GL_R16F,                                       // kR16SScaled
          GL_R16UI,                                      // kR16UInt
          GL_R16I,                                       // kR16SInt
          GL_R16F,                                       // kR16SFloat
                                                         //
          GL_RG16F,                                      // kR16G16UNorm
          GL_RG16F,                                      // kR16G16SNorm
          GL_RG16F,                                      // kR16G16UScaled
          GL_RG16F,                                      // kR16G16SScaled
          GL_RG16UI,                                     // kR16G16UInt
          GL_RG16I,                                      // kR16G16SInt
          GL_RG16F,                                      // kR16G16SFloat
                                                         //
          GL_RGB16F,                                     // kR16G16B16UNorm
          GL_RGB16F,                                     // kR16G16B16SNorm
          GL_RGB16F,                                     // kR16G16B16UScaled
          GL_RGB16F,                                     // kR16G16B16SScaled
          GL_RGB16UI,                                    // kR16G16B16UInt
          GL_RGB16I,                                     // kR16G16B16SInt
          GL_RGB16F,                                     // kR16G16B16SFloat
                                                         //
          GL_RGBA16F,                                    // kR16G16B16A16UNorm
          GL_RGBA16F,                                    // kR16G16B16A16SNorm
          GL_RGBA16F,                                    // kR16G16B16A16UScaled
          GL_RGBA16F,                                    // kR16G16B16A16SScaled
          GL_RGBA16UI,                                   // kR16G16B16A16UInt
          GL_RGBA16I,                                    // kR16G16B16A16SInt
          GL_RGBA16F,                                    // kR16G16B16A16SFloat
                                                         //
          GL_R32UI,                                      // kR32UInt
          GL_R32I,                                       // kR32SInt
          GL_R32F,                                       // kR32SFloat
                                                         //
          GL_RG32UI,                                     // kR32G32UInt
          GL_RG32I,                                      // kR32G32SInt
          GL_RG32F,                                      // kR32G32SFloat
                                                         //
          GL_RGB32UI,                                    // kR32G32B32UInt
          GL_RGB32I,                                     // kR32G32B32SInt
          GL_RGB32F,                                     // kR32G32B32SFloat
                                                         //
          GL_R32UI,                                      // kR32G32B32A32UInt
          GL_RGBA32I,                                    // kR32G32B32A32SInt
          GL_RGBA32F,                                    // kR32G32B32A32SFloat
                                                         //
          GL_NONE,                                       // kR64UInt
          GL_NONE,                                       // kR64SInt
          GL_NONE,                                       // kR64SFloat
                                                         //
          GL_NONE,                                       // kR64G64UInt
          GL_NONE,                                       // kR64G64SInt
          GL_NONE,                                       // kR64G64SFloat
                                                         //
          GL_NONE,                                       // kR64G64B64UInt
          GL_NONE,                                       // kR64G64B64SInt
          GL_NONE,                                       // kR64G64B64SFloat
                                                         //
          GL_NONE,                                       // kR64G64B64A64UInt
          GL_NONE,                                       // kR64G64B64A64SInt
          GL_NONE,                                       // kR64G64B64A64SFloat
                                                         //
          GL_R11F_G11F_B10F,                             // kB10G11R11UFloat
          GL_RGB9_E5,                                    // kE5B9G9R9UFloat
                                                         //
          GL_DEPTH_COMPONENT16,                          // kD16UNorm
          GL_DEPTH24_STENCIL8,                           // kX8D24UNorm
          GL_DEPTH_COMPONENT32F,                         // kD32SFloat
          GL_NONE,                                       // kS8UInt
          GL_NONE,                                       // kD16UNormS8UInt
          GL_DEPTH24_STENCIL8,                           // kD24UNormS8UInt
          GL_DEPTH32F_STENCIL8,                          // kD32SFloatS8UInt
                                                         //
          GL_COMPRESSED_RGB_S3TC_DXT1_EXT,               // kBC1RGBUNorm
          GL_COMPRESSED_SRGB_S3TC_DXT1_NV,               // kBC1RGBSrgb
          GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,              // kBC1RGBAUNorm
          GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV,         // kBC1RGBASrgb
          GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,              // kBC2UNorm
          GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV,         // kBC2Srgb
          GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,              // kBC3UNorm
          GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV,         // kBC3Srgb
          GL_NONE,                                       // kBC4UNorm
          GL_NONE,                                       // kBC4SNorm
          GL_NONE,                                       // kBC5UNorm
          GL_NONE,                                       // kBC5SNorm
          GL_NONE,                                       // kBC6HUFloat
          GL_NONE,                                       // kBC6HSFloat
          GL_NONE,                                       // kBC7UNorm
          GL_NONE,                                       // kBC7Srgb
                                                         //
          GL_COMPRESSED_RGB8_ETC2,                       // kEtc2R8G8B8UNorm
          GL_COMPRESSED_SRGB8_ETC2,                      // kEtc2R8G8B8Srgb
          GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,   // kEtc2R8G8B8A1UNorm
          GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,  // kEtc2R8G8B8A1Srgb
          GL_COMPRESSED_RGBA8_ETC2_EAC,                  // kEtc2R8G8B8A8UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,           // kEtc2R8G8B8A8Srgb
                                                         //
          GL_COMPRESSED_R11_EAC,                         // kEacR11UNorm
          GL_COMPRESSED_SIGNED_R11_EAC,                  // kEacR11SNorm
          GL_COMPRESSED_RG11_EAC,                        // kEacR11G11UNorm
          GL_COMPRESSED_SIGNED_RG11_EAC,                 // kEacR11G11SNorm
                                                         //
          GL_COMPRESSED_RGBA_ASTC_4x4,                   // kAstc4x4UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4,           // kAstc4x4Srgb
          GL_COMPRESSED_RGBA_ASTC_5x4,                   // kAstc5x4UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4,           // kAstc5x4Srgb
          GL_COMPRESSED_RGBA_ASTC_5x5,                   // kAstc5x5UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5,           // kAstc5x5Srgb
          GL_COMPRESSED_RGBA_ASTC_6x5,                   // kAstc6x5UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5,           // kAstc6x5Srgb
          GL_COMPRESSED_RGBA_ASTC_6x6,                   // kAstc6x6UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6,           // kAstc6x6Srgb
          GL_COMPRESSED_RGBA_ASTC_8x5,                   // kAstc8x5UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5,           // kAstc8x5Srgb
          GL_COMPRESSED_RGBA_ASTC_8x6,                   // kAstc8x6UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6,           // kAstc8x6Srgb
          GL_COMPRESSED_RGBA_ASTC_8x8,                   // kAstc8x8UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8,           // kAstc8x8Srgb
          GL_COMPRESSED_RGBA_ASTC_10x5,                  // kAstc10x5UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5,          // kAstc10x5Srgb
          GL_COMPRESSED_RGBA_ASTC_10x6,                  // kAstc10x6UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6,          // kAstc10x6Srgb
          GL_COMPRESSED_RGBA_ASTC_10x8,                  // kAstc10x8UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8,          // kAstc10x8Srgb
          GL_COMPRESSED_RGBA_ASTC_10x10,                 // kAstc10x10UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10,         // kAstc10x10Srgb
          GL_COMPRESSED_RGBA_ASTC_12x10,                 // kAstc12x10UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10,         // kAstc12x10Srgb
          GL_COMPRESSED_RGBA_ASTC_12x12,                 // kAstc12x12UNorm
          GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12,         // kAstc12x12Srgb
      });
  return kInternalFormats.Find(pixel_format.unique_id());
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
