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

#ifndef XRTL_GFX_ES3_ES3_COMMON_H_
#define XRTL_GFX_ES3_ES3_COMMON_H_

#if defined(XRTL_PLATFORM_WINDOWS)
#include "xrtl/port/windows/base/windows.h"
#endif  // XRTL_PLATFORM_WINDOWS

// GLAD generated loader headers.
// These contain all of the OpenGL ES 3 symbols and should be used instead of
// the khronos headers.
// This must come after windows.h is included.
#include <glad/glad.h>  // NOLINT(build/include_order)

// Windows #define's this, which messes with our implementation.
#undef MemoryBarrier

#include "xrtl/base/logging.h"

// Uncomment the following line or manually define DEBUG_GL in command line to
// enable DCHECK_NO_GL_ERROR() to check GL errors.
// #define DEBUG_GL

// Checks that glGetError() is clean.
// This has a significant performance overhead and must not be normally enabled.
#if defined(DEBUG_GL)
#define DCHECK_NO_GL_ERROR() DCHECK_EQ(GL_NO_ERROR, glGetError())
#else
#define DCHECK_NO_GL_ERROR()
#endif  // DEBUG_GL

// Checks that the specified GL context is current.
#if defined(DEBUG_GL)
#define DCHECK_CONTEXT_IS_CURRENT(context) \
  DCHECK_TRUE(context && context->IsCurrent())
#else
#define DCHECK_CONTEXT_IS_CURRENT(context)
#endif  // DEBUG_GL

namespace xrtl {
namespace gfx {
namespace es3 {

// TODO(benvanik): use device limits resource_set_count.
constexpr int kMaxResourceSetCount = 4;

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_COMMON_H_
