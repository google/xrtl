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

#ifndef XRTL_GFX_ES3_ES3_PIXEL_FORMAT_H_
#define XRTL_GFX_ES3_ES3_PIXEL_FORMAT_H_

#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/pixel_format.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// Parameters for GL texture functions derived from our pixel formats.
struct ES3TextureParams {
  // 'internalFormat' parameter.
  GLenum internal_format = GL_NONE;
  // 'format' parameter.
  GLenum format = GL_NONE;
  // 'type' parameter.
  GLenum type = GL_NONE;

  ES3TextureParams() = default;
  ES3TextureParams(GLenum internal_format, GLenum format, GLenum type)
      : internal_format(internal_format), format(format), type(type) {}
};

// Converts a PixelFormat to a set of GL texture parameter info.
// Returns false if the PixelFormat cannot be represented in GL.
bool ConvertPixelFormatToTextureParams(PixelFormat pixel_format,
                                       ES3TextureParams* out_texture_params);

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_PIXEL_FORMAT_H_
