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

#ifndef XRTL_PORT_WINDOWS_GFX_ES3_WGL_PLATFORM_CONTEXT_H_
#define XRTL_PORT_WINDOWS_GFX_ES3_WGL_PLATFORM_CONTEXT_H_

#include <chrono>
#include <mutex>
#include <vector>

#include "xrtl/base/macros.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/port/windows/base/windows.h"

// These must come after windows.h is included:
#include <glad/glad.h>      // NOLINT(build/include_order)
#include <glad/glad_wgl.h>  // NOLINT(build/include_order)

namespace xrtl {
namespace gfx {
namespace es3 {

// WGL-based GL context.
class WGLPlatformContext : public ES3PlatformContext {
 public:
  WGLPlatformContext();
  ~WGLPlatformContext();

  bool Initialize(HDC native_display, HWND native_window,
                  ref_ptr<ES3PlatformContext> share_group);

  void* native_handle() const override { return glrc_; }

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

  // Creates a dummy window that can be used when a HWND/HDC is required.
  HWND CreateDummyWindow();

  bool InitializeWGL(HDC hdc);
  bool InitializeContext(ref_ptr<ES3PlatformContext> share_group);
  void InitializeDebugging();

  bool is_headless_ = false;
  HDC native_display_ = nullptr;
  HWND native_window_ = nullptr;
  HGLRC glrc_ = nullptr;

  bool is_robust_access_supported_ = false;
  bool has_lost_context_ = false;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_PORT_WINDOWS_GFX_ES3_WGL_PLATFORM_CONTEXT_H_
