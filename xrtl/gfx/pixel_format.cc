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

#include "xrtl/gfx/pixel_format.h"

namespace xrtl {
namespace gfx {

size_t PixelFormat::ComputeCompressedDataSize(int width, int height) const {
  switch (packing_format_) {
    case PixelPacking::kBC1:
    case PixelPacking::kBC4:
    case PixelPacking::kEtc1:
      return 8 * ((width + 3) / 4) * ((height + 3) / 4);
    case PixelPacking::kBC2:
    case PixelPacking::kBC3:
    case PixelPacking::kBC5:
    case PixelPacking::kBC6:
    case PixelPacking::kBC7:
    case PixelPacking::kEac:
      return 16 * ((width + 3) / 4) * ((height + 3) / 4);
    case PixelPacking::kEtc2:
      if (component_bits_a_ < 8) {
        return 8 * ((width + 3) / 4) * ((height + 3) / 4);
      } else {
        return 16 * ((width + 3) / 4) * ((height + 3) / 4);
      }
    case PixelPacking::kAstc: {
      struct FootprintSize {
        int width;
        int height;
      };
      static const PixelFormatTable<FootprintSize, PixelFormats::kAstc4x4UNorm,
                                    PixelFormats::kAstc12x12Srgb, 2>
          kAstcFootprintSizes({
              {4, 4},    // kAstc4x4UNorm / kAstc4x4Srgb
              {5, 4},    // kAstc5x4UNorm / kAstc5x4Srgb
              {5, 5},    // kAstc5x5UNorm / kAstc5x5Srgb
              {6, 5},    // kAstc6x5UNorm / kAstc6x5Srgb
              {6, 6},    // kAstc6x6UNorm / kAstc6x6Srgb
              {8, 5},    // kAstc8x5UNorm / kAstc8x5Srgb
              {8, 6},    // kAstc8x6UNorm / kAstc8x6Srgb
              {8, 8},    // kAstc8x8UNorm / kAstc8x8Srgb
              {10, 5},   // kAstc10x5UNorm / kAstc10x5Srgb
              {10, 6},   // kAstc10x6UNorm / kAstc10x6Srgb
              {10, 8},   // kAstc10x8UNorm / kAstc10x8Srgb
              {10, 10},  // kAstc10x10UNorm / kAstc10x10Srgb
              {12, 10},  // kAstc12x10UNorm / kAstc12x10Srgb
              {12, 12},  // kAstc12x12UNorm / kAstc12x12Srgb
          });
      auto footprint = kAstcFootprintSizes.Find(unique_id_);
      // Each MxN block of pixels requires 16 bytes. Round up image dimensions
      // to block size.
      return 16 * ((width + footprint.width - 1) / footprint.width) *
             ((height + footprint.height - 1) / footprint.height);
    }
    default:
      DCHECK(false);
      return 0;
  }
}

}  // namespace gfx
}  // namespace xrtl
