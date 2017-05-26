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

#include "xrtl/port/common/gfx/es3/egl_strings.h"

#include <glad/glad_egl.h>

namespace xrtl {
namespace gfx {
namespace es3 {

const char* GetEglErrorName(int error) {
  switch (error) {
    case EGL_SUCCESS:
      return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED:
      return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
      return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
      return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
      return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONTEXT:
      return "EGL_BAD_CONTEXT";
    case EGL_BAD_CONFIG:
      return "EGL_BAD_CONFIG";
    case EGL_BAD_CURRENT_SURFACE:
      return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
      return "EGL_BAD_DISPLAY";
    case EGL_BAD_SURFACE:
      return "EGL_BAD_SURFACE";
    case EGL_BAD_MATCH:
      return "EGL_BAD_MATCH";
    case EGL_BAD_PARAMETER:
      return "EGL_BAD_PARAMETER";
    case EGL_BAD_NATIVE_PIXMAP:
      return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
      return "EGL_BAD_NATIVE_WINDOW";
    case EGL_CONTEXT_LOST:
      return "EGL_CONTEXT_LOST";
    default:
      return "UNKNOWN";
  }
}

const char* GetEglErrorDescription(int error) {
  switch (error) {
    case EGL_SUCCESS:
      return "The last function succeeded without error.";
    case EGL_NOT_INITIALIZED:
      return "EGL is not initialized, or could not be initialized, for the "
             "specified EGL display connection.";
    case EGL_BAD_ACCESS:
      return "EGL cannot access a requested resource (for example a context is "
             "bound in another thread).";
    case EGL_BAD_ALLOC:
      return "EGL failed to allocate resources for the requested operation.";
    case EGL_BAD_ATTRIBUTE:
      return "An unrecognized attribute or attribute value was passed in the "
             "attribute list.";
    case EGL_BAD_CONTEXT:
      return "An EGLContext argument does not name a valid EGL rendering "
             "context.";
    case EGL_BAD_CONFIG:
      return "An EGLConfig argument does not name a valid EGL frame buffer "
             "configuration.";
    case EGL_BAD_CURRENT_SURFACE:
      return "The current surface of the calling thread is a window, pixel "
             "buffer or pixmap that is no longer valid.";
    case EGL_BAD_DISPLAY:
      return "An EGLDisplay argument does not name a valid EGL display "
             "connection.";
    case EGL_BAD_SURFACE:
      return "An EGLSurface argument does not name a valid surface (window, "
             "pixel buffer or pixmap) configured for GL rendering.";
    case EGL_BAD_MATCH:
      return "Arguments are inconsistent (for example, a valid context "
             "requires buffers not supplied by a valid surface).";
    case EGL_BAD_PARAMETER:
      return "One or more argument values are invalid.";
    case EGL_BAD_NATIVE_PIXMAP:
      return "A NativePixmapType argument does not refer to a valid native "
             "pixmap.";
    case EGL_BAD_NATIVE_WINDOW:
      return "A NativeWindowType argument does not refer to a valid native "
             "window.";
    case EGL_CONTEXT_LOST:
      return "A power management event has occurred. The application must "
             "destroy all contexts and reinitialise OpenGL ES state and "
             "objects to continue rendering.";
    default:
      return "An unknown error occurred.";
  }
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
