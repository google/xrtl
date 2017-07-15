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

#ifndef XRTL_PORT_COMMON_GFX_ES3_EGL_PLATFORM_CONTEXT_H_
#define XRTL_PORT_COMMON_GFX_ES3_EGL_PLATFORM_CONTEXT_H_

#include <glad/glad.h>
#include <glad/glad_egl.h>

#include <chrono>
#include <mutex>
#include <vector>

#include "xrtl/base/macros.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

enum class ConfigRequestFlag {
  // Requires configs to have conformant OpenGL ES 2 support.
  kOpenGLES2 = 1 << 0,
  // Requires configs to have conformant OpenGL ES 3 support.
  // If both kOpenGLES2 and kOpenGLES3 are set 3 will be considered optional.
  kOpenGLES3 = 1 << 1,
  // Requires a P-buffer output surface type.
  kPBufferSurfaceType = 1 << 2,
  // Requires a native window surface type.
  kWindowSurfaceType = 1 << 3,
  // Requires configs to have an alpha channel of 8 bits per pixel.
  kAlpha8Required = 1 << 4,
  // Requires configs to have a depth channel of at least 16 bits per pixel.
  kDepthGE16Required = 1 << 5,
  // Requires configs to have a stencil buffer of at least 8 bits per pixel.
  kStencilGE8Required = 1 << 6,
};
XRTL_BITMASK(ConfigRequestFlag);

// EGL-based GL context.
class EGLPlatformContext : public ES3PlatformContext {
 public:
  EGLPlatformContext();
  ~EGLPlatformContext();

  bool Initialize(EGLNativeDisplayType native_display,
                  EGLNativeWindowType native_window,
                  ref_ptr<ES3PlatformContext> share_group);

  void* native_handle() const override { return egl_context_; }

  bool IsCurrent() override;
  bool MakeCurrent() override;
  void ClearCurrent() override;

  void Flush() override;
  void Finish() override;

  RecreateSurfaceResult RecreateSurface(Size2D size_hint) override;
  Size2D QuerySize() override;
  void SetSwapBehavior(SwapBehavior swap_behavior) override;
  bool SwapBuffers(std::chrono::milliseconds present_time_utc_millis) override;

  void* GetExtensionProc(const char* extension_name,
                         const char* proc_name) override;

 private:
  friend class ES3PlatformContext;

  bool InitializeContext(ref_ptr<ES3PlatformContext> share_group);

  // Attempts to find a config matching the required attributes (such as
  // color depth).
  // Returns false if no matching config is found.
  bool ChooseBestConfig(EGLDisplay egl_display,
                        ConfigRequestFlag config_request_flags,
                        EGLConfig* out_egl_config);
  // Returns true if the given config meets the minimum requirements.
  bool ValidateConfig(EGLDisplay egl_display, EGLConfig egl_config,
                      ConfigRequestFlag config_request_flags);
  // Sorts the given list of configs such that the first element is the 'best'.
  void SortConfigs(EGLDisplay egl_display, std::vector<EGLConfig>* egl_configs,
                   ConfigRequestFlag config_request_flags);
  // Dumps all config attributes to LOG.
  void DumpConfig(EGLDisplay egl_display, EGLConfig egl_config);

  // Finishes all context operations before shutting down.
  void FinishOnShutdown();

  EGLDisplay egl_display_ = EGL_NO_DISPLAY;
  EGLConfig egl_config_ = nullptr;
  EGLContext egl_context_ = EGL_NO_CONTEXT;
  EGLSurface egl_surface_ = EGL_NO_SURFACE;

  EGLNativeDisplayType native_display_{0};
  EGLNativeWindowType native_window_{0};

  // EGL implementation has the EGL_KHR_surfaceless_context extension.
  bool supports_surfaceless_context_ = false;
  // EGL implementation has the EGL_MESA_configless_context extension.
  bool supports_configless_context_ = false;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_GFX_ES3_EGL_PLATFORM_CONTEXT_H_
