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

bool ConvertPixelFormatToTextureParams(PixelFormat pixel_format,
                                       ES3TextureParams* out_texture_params) {
  static const ES3TextureParams kTable[] = {
      {GL_NONE, GL_NONE, GL_NONE},                       // kUndefined
                                                         //
      {GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE},             // kR4G4B4A4UNorm
      {GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},      // kR5G6B5UNorm
      {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1},  // kR5G5B5A1UNorm
                                                         //
      {GL_R8, GL_RED, GL_UNSIGNED_BYTE},                 // kR8UNorm
      {GL_R8_SNORM, GL_RED, GL_BYTE},                    // kR8SNorm
      {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE},       // kR8UInt
      {GL_R8I, GL_RED_INTEGER, GL_BYTE},                 // kR8SInt
                                                         //
      {GL_RG8, GL_RG, GL_UNSIGNED_BYTE},                 // kR8G8UNorm
      {GL_RG8_SNORM, GL_RG, GL_BYTE},                    // kR8G8SNorm
      {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE},       // kR8G8UInt
      {GL_RG8I, GL_RG_INTEGER, GL_BYTE},                 // kR8G8SInt
                                                         //
      {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},             // kR8G8B8A8UNorm
      {GL_RGBA8_SNORM, GL_RGBA, GL_BYTE},                // kR8G8B8A8SNorm
      {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE},   // kR8G8B8A8UInt
      {GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE},             // kR8G8B8A8SInt
      {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},      // kR8G8B8A8Srgb
                                                         //
      {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},             // kB8G8R8A8UNorm
      {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},      // kB8G8R8A8Srgb
                                                         //
      {GL_RGB10_A2, GL_RGBA,                             //
       GL_UNSIGNED_INT_2_10_10_10_REV},                  // kA2B10G10R10UNorm
      {GL_RGB10_A2, GL_RGBA,                             //
       GL_UNSIGNED_INT_2_10_10_10_REV},                  // kA2B10G10R10SNorm
      {GL_RGB10_A2UI, GL_RGBA_INTEGER,                   //
       GL_UNSIGNED_INT_2_10_10_10_REV},                  // kA2B10G10R10UInt
      {GL_RGB10_A2UI, GL_RGBA_INTEGER,                   //
       GL_UNSIGNED_INT_2_10_10_10_REV},                  // kA2B10G10R10SInt
                                                         //
      {GL_R16F, GL_RED, GL_HALF_FLOAT},                  // kR16UNorm
      {GL_R16F, GL_RED, GL_HALF_FLOAT},                  // kR16SNorm
      {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT},     // kR16UInt
      {GL_R16I, GL_RED_INTEGER, GL_SHORT},               // kR16SInt
      {GL_R16F, GL_RED, GL_HALF_FLOAT},                  // kR16SFloat
                                                         //
      {GL_RG16F, GL_RG, GL_HALF_FLOAT},                  // kR16G16UNorm
      {GL_RG16F, GL_RG, GL_HALF_FLOAT},                  // kR16G16SNorm
      {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT},     // kR16G16UInt
      {GL_RG16I, GL_RG_INTEGER, GL_SHORT},               // kR16G16SInt
      {GL_RG16F, GL_RG, GL_HALF_FLOAT},                  // kR16G16SFloat
                                                         //
      {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},              // kR16G16B16A16UNorm
      {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},              // kR16G16B16A16SNorm
      {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT},  // kR16G16B16A16UInt
      {GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT},            // kR16G16B16A16SInt
      {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},               // kR16G16B16A16SFloat
                                                          //
      {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT},        // kR32UInt
      {GL_R32I, GL_RED_INTEGER, GL_INT},                  // kR32SInt
      {GL_R32F, GL_RED, GL_FLOAT},                        // kR32SFloat
                                                          //
      {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT},        // kR32G32UInt
      {GL_RG32I, GL_RG_INTEGER, GL_INT},                  // kR32G32SInt
      {GL_RG32F, GL_RG, GL_FLOAT},                        // kR32G32SFloat
                                                          //
      {GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT},      // kR32G32B32UInt
      {GL_RGB32I, GL_RGB_INTEGER, GL_INT},                // kR32G32B32SInt
      {GL_RGB32F, GL_RGB, GL_FLOAT},                      // kR32G32B32SFloat
                                                          //
      {GL_R32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT},       // kR32G32B32A32UInt
      {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT},              // kR32G32B32A32SInt
      {GL_RGBA32F, GL_RGBA, GL_FLOAT},                    // kR32G32B32A32SFloat
                                                          //
      {GL_R11F_G11F_B10F, GL_RGB,                         //
       GL_UNSIGNED_INT_10F_11F_11F_REV},                  // kB10G11R11UFloat
      {GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV},  // kE5B9G9R9UFloat
                                                          //
      {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT},  // kD32SFloat
      {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,                 //
       GL_UNSIGNED_INT_24_8},                                 // kD24UNormS8UInt
      {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL,                //
       GL_FLOAT_32_UNSIGNED_INT_24_8_REV},                   // kD32SFloatS8UInt
                                                             //
      {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_RGBA, GL_NONE},  // kBC1RGBAUNorm
      {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV, GL_RGBA,       //
       GL_NONE},                                             // kBC1RGBASrgb
      {GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_RGBA, GL_NONE},  // kBC2UNorm
      {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV, GL_RGBA, GL_NONE},  // kBC2Srgb
      {GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_RGBA, GL_NONE},       // kBC3UNorm
      {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV, GL_RGBA, GL_NONE},  // kBC3Srgb
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC4UNorm
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC4SNorm
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC5UNorm
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC5SNorm
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC6HUFloat
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC6HSFloat
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC7UNorm
      {GL_NONE, GL_NONE, GL_NONE},                                // kBC7Srgb
                                                                  //
      {GL_COMPRESSED_RGB8_ETC2, GL_RGB, GL_NONE},   // kEtc2R8G8B8UNorm
      {GL_COMPRESSED_SRGB8_ETC2, GL_RGB, GL_NONE},  // kEtc2R8G8B8Srgb
      {GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_RGBA,  //
       GL_NONE},  // kEtc2R8G8B8A1UNorm
      {GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, GL_RGBA,  //
       GL_NONE},                                         // kEtc2R8G8B8A1Srgb
      {GL_COMPRESSED_RGBA8_ETC2_EAC, GL_RGBA, GL_NONE},  // kEtc2R8G8B8A8UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, GL_RGBA,     //
       GL_NONE},                                         // kEtc2R8G8B8A8Srgb
                                                         //
      {GL_COMPRESSED_R11_EAC, GL_RED, GL_NONE},          // kEacR11UNorm
      {GL_COMPRESSED_SIGNED_R11_EAC, GL_RED, GL_NONE},   // kEacR11SNorm
      {GL_COMPRESSED_RG11_EAC, GL_RG, GL_NONE},          // kEacR11G11UNorm
      {GL_COMPRESSED_SIGNED_RG11_EAC, GL_RED, GL_NONE},  // kEacR11G11SNorm
                                                         //
      {GL_COMPRESSED_RGBA_ASTC_4x4, GL_RGBA, GL_NONE},   // kAstc4x4UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4, GL_RGBA, GL_NONE},  // kAstc4x4Srgb
      {GL_COMPRESSED_RGBA_ASTC_5x4, GL_RGBA, GL_NONE},          // kAstc5x4UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4, GL_RGBA, GL_NONE},  // kAstc5x4Srgb
      {GL_COMPRESSED_RGBA_ASTC_5x5, GL_RGBA, GL_NONE},          // kAstc5x5UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5, GL_RGBA, GL_NONE},  // kAstc5x5Srgb
      {GL_COMPRESSED_RGBA_ASTC_6x5, GL_RGBA, GL_NONE},          // kAstc6x5UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5, GL_RGBA, GL_NONE},  // kAstc6x5Srgb
      {GL_COMPRESSED_RGBA_ASTC_6x6, GL_RGBA, GL_NONE},          // kAstc6x6UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6, GL_RGBA, GL_NONE},  // kAstc6x6Srgb
      {GL_COMPRESSED_RGBA_ASTC_8x5, GL_RGBA, GL_NONE},          // kAstc8x5UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5, GL_RGBA, GL_NONE},  // kAstc8x5Srgb
      {GL_COMPRESSED_RGBA_ASTC_8x6, GL_RGBA, GL_NONE},          // kAstc8x6UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6, GL_RGBA, GL_NONE},  // kAstc8x6Srgb
      {GL_COMPRESSED_RGBA_ASTC_8x8, GL_RGBA, GL_NONE},          // kAstc8x8UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8, GL_RGBA, GL_NONE},  // kAstc8x8Srgb
      {GL_COMPRESSED_RGBA_ASTC_10x5, GL_RGBA, GL_NONE},   // kAstc10x5UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5, GL_RGBA,     //
       GL_NONE},                                          // kAstc10x5Srgb
      {GL_COMPRESSED_RGBA_ASTC_10x6, GL_RGBA, GL_NONE},   // kAstc10x6UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6, GL_RGBA,     //
       GL_NONE},                                          // kAstc10x6Srgb
      {GL_COMPRESSED_RGBA_ASTC_10x8, GL_RGBA, GL_NONE},   // kAstc10x8UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8, GL_RGBA,     //
       GL_NONE},                                          // kAstc10x8Srgb
      {GL_COMPRESSED_RGBA_ASTC_10x10, GL_RGBA, GL_NONE},  // kAstc10x10UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10, GL_RGBA,    //
       GL_NONE},                                          // kAstc10x10Srgb
      {GL_COMPRESSED_RGBA_ASTC_12x10, GL_RGBA, GL_NONE},  // kAstc12x10UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10, GL_RGBA,    //
       GL_NONE},                                          // kAstc12x10Srgb
      {GL_COMPRESSED_RGBA_ASTC_12x12, GL_RGBA, GL_NONE},  // kAstc12x12UNorm
      {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12, GL_RGBA,    //
       GL_NONE},                                          // kAstc12x12Srgb
  };
  static const PixelFormatTable<ES3TextureParams, PixelFormats::kUndefined,
                                PixelFormats::kAstc12x12Srgb>
      kLookupTable(kTable);
  const auto& texture_params = kLookupTable.Find(pixel_format.unique_id());
  if (texture_params.internal_format == GL_NONE) {
    return false;
  }
  *out_texture_params = texture_params;
  return true;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
