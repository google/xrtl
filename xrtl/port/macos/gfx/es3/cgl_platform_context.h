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

#ifndef XRTL_PORT_MACOS_GFX_ES3_CGL_PLATFORM_CONTEXT_H_
#define XRTL_PORT_MACOS_GFX_ES3_CGL_PLATFORM_CONTEXT_H_

#include <chrono>
#include <mutex>
#include <vector>

#include "xrtl/base/macros.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

// OpenGL.h must be included before glad.h.
#include <OpenGL/OpenGL.h>  // NOLINT(build/include_order)
#include <glad/glad.h>      // NOLINT(build/include_order)

namespace xrtl {
namespace gfx {
namespace es3 {

// CGL-based GL context.
class CGLPlatformContext : public ES3PlatformContext {
 public:
  CGLPlatformContext();
  ~CGLPlatformContext();

  bool Initialize(void* native_display, void* native_window,
                  ref_ptr<ES3PlatformContext> share_group);

  void* native_handle() const override { return context_; }

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

  bool InitializeCGL(void* native_display);
  bool InitializeContext(ref_ptr<ES3PlatformContext> share_group);
  void InitializeDebugging();

  bool is_headless_ = false;
  CGLPixelFormatObj pixel_format_ = nullptr;
  CGLContextObj context_ = nullptr;

  bool has_lost_context_ = false;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_PORT_MACOS_GFX_ES3_CGL_PLATFORM_CONTEXT_H_
