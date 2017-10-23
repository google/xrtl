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

#include "xrtl/port/macos/gfx/es3/cgl_platform_context.h"

#include <dlfcn.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "absl/base/call_once.h"
#include "xrtl/base/debugging.h"
#include "xrtl/base/flags.h"
#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace es3 {

namespace {

// From:
// http://mirror.informatimago.com/next/developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL/chap5/chapter_5_section_41.html#//apple_ref/doc/uid/TP30000136/BDJHHIDE
std::string GetCglErrorName(CGLError error) {
  switch (error) {
    case kCGLBadAttribute:
      return "kCGLBadAttribute";
    case kCGLBadProperty:
      return "kCGLBadProperty";
    case kCGLBadPixelFormat:
      return "kCGLBadPixelFormat";
    case kCGLBadRendererInfo:
      return "kCGLBadRendererInfo";
    case kCGLBadContext:
      return "kCGLBadContext";
    case kCGLBadDrawable:
      return "kCGLBadDrawable";
    case kCGLBadDisplay:
      return "kCGLBadDisplay";
    case kCGLBadState:
      return "kCGLBadState";
    case kCGLBadValue:
      return "kCGLBadValue";
    case kCGLBadMatch:
      return "kCGLBadMatch";
    case kCGLBadEnumeration:
      return "kCGLBadEnumeration";
    case kCGLBadOffScreen:
      return "kCGLBadOffScreen";
    case kCGLBadFullScreen:
      return "kCGLBadFullScreen";
    case kCGLBadWindow:
      return "kCGLBadWindow";
    case kCGLBadAddress:
      return "kCGLBadAddress";
    case kCGLBadCodeModule:
      return "kCGLBadCodeModule";
    case kCGLBadAlloc:
      return "kCGLBadAlloc";
    case kCGLBadConnection:
      return "kCGLBadConnection";
    default:
      return "UNKNOWN";
  }
}

std::string GetCglErrorDescription(CGLError error) {
  return std::string(CGLErrorString(error));
}

void* LoadOpenGLFunction(const char* proc_name) {
  return dlsym(RTLD_DEFAULT, proc_name);
}

}  // namespace

ref_ptr<ES3PlatformContext> ES3PlatformContext::Create(
    void* native_display, void* native_window,
    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("ES3PlatformContext#Create");

  auto platform_context = make_ref<CGLPlatformContext>();

  if (!platform_context->Initialize(reinterpret_cast<void*>(native_display),
                                    reinterpret_cast<void*>(native_window),
                                    std::move(share_group))) {
    LOG(ERROR) << "Unable to initialize the CGL platform context";
    return nullptr;
  }

  return platform_context;
}

CGLPlatformContext::CGLPlatformContext() = default;

CGLPlatformContext::~CGLPlatformContext() {
  WTF_SCOPE0("CGLPlatformContext#dtor");

  // Finish all context operations.
  if (context_) {
    if (MakeCurrent()) {
      Finish();
    }
    ClearCurrent();
  }

  if (context_) {
    CGLDestroyContext(context_);
    context_ = nullptr;
  }
  if (pixel_format_) {
    CGLDestroyPixelFormat(pixel_format_);
    pixel_format_ = nullptr;
  }
}

bool CGLPlatformContext::Initialize(void* native_display, void* native_window,
                                    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("CGLPlatformContext#Initialize");

  // Ensure CGL is initialized. May have been done elsewhere.
  if (!InitializeCGL(native_display)) {
    LOG(ERROR) << "Failed to initialize CGL; cannot create context";
    return false;
  }

  // Setup a pixel format, even for headless contexts.
  std::vector<CGLPixelFormatAttribute> attributes;

  // Require hardware acceleration.
  attributes.push_back(kCGLPFAAccelerated);

  // TODO(benvanik): support this by listening for display changes.
  // Allow dual-GPU mode switching.
  // attributes.push_back(kCGLPFAAllowOfflineRenderers);

  // Specify GL profile (3.2 for now, which is close to ES3).
  attributes.push_back(kCGLPFAOpenGLProfile);
  attributes.push_back(
      static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_3_2_Core));

  // Limit to target display.
  if (!is_headless_) {
    // TODO(benvanik): set kCGLPFADisplayMask.
  }

  // Setup default backbuffer.
  if (!is_headless_) {
    // TODO(benvanik): set color/depth/etc.
    // TODO(benvanik): allow swapchain to specify a format?
  }

  // NUL list terminator.
  attributes.push_back(static_cast<CGLPixelFormatAttribute>(0));

  // Query pixel formats that match our attributes.
  GLint pixel_format_count = 0;
  CGLError error = CGLChoosePixelFormat(attributes.data(), &pixel_format_,
                                        &pixel_format_count);
  if (error) {
    LOG(ERROR) << "CGLChoosePixelFormat failed: " << GetCglErrorName(error)
               << ": " << GetCglErrorDescription(error);
    return false;
  }

  // Grab the share group context, if it exists.
  CGLContextObj share_context = nullptr;
  if (share_group) {
    share_context =
        reinterpret_cast<CGLContextObj>(share_group->native_handle());
  }

  // Create the context.
  error = CGLCreateContext(pixel_format_, share_context, &context_);
  if (error) {
    LOG(ERROR) << "CGLCreateContext failed: " << GetCglErrorName(error) << ": "
               << GetCglErrorDescription(error);
    return false;
  }

  // Try to make the context current as it may be invalid but we won't know
  // until the first attempt. Catching the error here makes it easier to find.
  ES3PlatformContext::ExclusiveLock context_lock(this);
  if (!context_lock.is_held()) {
    LOG(ERROR) << "Initial MakeCurrent failed, aborting initialization";
    return false;
  }

  // Setup GL functions. We only need to do this once.
  // NOTE: GLAD is not thread safe! We must only be calling this from a single
  //       thread.
  static absl::once_flag load_gles2_flag;
  static std::atomic<bool> loaded_gles2{false};
  absl::call_once(load_gles2_flag, []() {
    loaded_gles2 = gladLoadGLES2Loader(LoadOpenGLFunction);
  });
  if (!loaded_gles2) {
    LOG(ERROR) << "Failed to load GL ES dynamic functions";
    return false;
  }

  // Grab GL info.
  static absl::once_flag log_gl_flag;
  absl::call_once(log_gl_flag, []() {
    LOG(INFO) << "GL initialized successfully:" << std::endl
              << "GL vendor: " << glGetString(GL_VENDOR) << std::endl
              << "GL renderer: " << glGetString(GL_RENDERER) << std::endl
              << "GL version: " << glGetString(GL_VERSION) << std::endl;
    VLOG(1) << "GL extensions:";
    GLint extension_count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
    for (GLint i = 0; i < extension_count; ++i) {
      VLOG(1) << "  " << glGetStringi(GL_EXTENSIONS, i);
    }
  });

  // Query limits and other information from the context.
  if (!InitializeLimits()) {
    LOG(ERROR) << "Failed to initialize platform context limits";
    return false;
  }

  // Query available extensions and setup the enable state tracking.
  if (!InitializeExtensions()) {
    LOG(ERROR) << "Failed to initialize platform context extension support";
    return false;
  }

  // Reset context. We'll re-bind it later as needed.
  // We want to make sure that if we are going to use the context on another
  // thread we haven't left it dangling here.
  context_lock.reset();

  // Initialize the target surface (if not offscreen).
  if (!is_headless_) {
    if (RecreateSurface({0, 0}) != RecreateSurfaceResult::kSuccess) {
      LOG(ERROR) << "Unable to create window surface";
      return false;
    }
  }

  return true;
}

bool CGLPlatformContext::InitializeCGL(void* native_display) {
  WTF_SCOPE0("CGLPlatformContext#InitializeCGL");

  // Attempt to load CGL.
  static absl::once_flag load_flag;
  static std::atomic<bool> load_result{false};
  absl::call_once(load_flag, [this]() {
    // TODO(benvanik): some dynamic magic?
    load_result = true;
  });

  return load_result;
}

bool CGLPlatformContext::IsCurrent() {
  if (context_) {
    return CGLGetCurrentContext() == context_;
  } else {
    return false;
  }
}

bool CGLPlatformContext::MakeCurrent() {
  WTF_SCOPE0("CGLPlatformContext#MakeCurrent");

  DCHECK(context_ != nullptr);

  if (has_lost_context_) {
    // We've already lost our context - nothing to do.
    return false;
  }

  if (IsCurrent()) {
    // No-op.
    return true;
  }

  CGLError error = CGLSetCurrentContext(context_);
  if (error) {
    LOG(ERROR) << "CGLSetCurrentContext failed: " << GetCglErrorName(error)
               << ": " << GetCglErrorDescription(error);
    return false;
  }

  return true;
}

void CGLPlatformContext::ClearCurrent() {
  WTF_SCOPE0("CGLPlatformContext#ClearCurrent");
  CGLSetCurrentContext(nullptr);
}

void CGLPlatformContext::Flush() {
  WTF_SCOPE0("CGLPlatformContext#Flush");
  DCHECK(IsCurrent());
  glFlush();
}

void CGLPlatformContext::Finish() {
  WTF_SCOPE0("CGLPlatformContext#Finish");
  DCHECK(IsCurrent());
  if (glFinish) {
    glFinish();
  }
}

CGLPlatformContext::RecreateSurfaceResult CGLPlatformContext::RecreateSurface(
    Size2D size_hint) {
  WTF_SCOPE0("CGLPlatformContext#RecreateSurface");
  // TODO(benvanik): resize framebuffer/layer.
  return RecreateSurfaceResult::kSuccess;
}

Size2D CGLPlatformContext::QuerySize() {
  DCHECK(context_ != nullptr);

  if (is_headless_) {
    // No-op.
    return {0, 0};
  }

  // TODO(benvanik): return layer dimensions.
  return {};
}

void CGLPlatformContext::SetSwapBehavior(SwapBehavior swap_behavior) {
  DCHECK(context_ != nullptr);
  GLint param_value = 0;
  switch (swap_behavior) {
    case SwapBehavior::kImmediate:
      param_value = 0;
      break;
    case SwapBehavior::kSynchronize:
      param_value = 1;
      break;
    case SwapBehavior::kSynchronizeAndTear:
      // Not available, AFAICT.
      param_value = 1;
      break;
  }
  CGLError error = CGLSetParameter(context_, kCGLCPSwapInterval, &param_value);
  if (error) {
    LOG(ERROR) << "CGLSetParameter of swap interval failed: "
               << GetCglErrorName(error) << ": "
               << GetCglErrorDescription(error);
    return;
  }
}

bool CGLPlatformContext::SwapBuffers(
    std::chrono::milliseconds present_time_utc_millis) {
  if (is_headless_) {
    // No-op.
    return true;
  }

  // TODO(benvanik): flush to layer.
  return false;
}

void* CGLPlatformContext::GetExtensionProc(const char* extension_name,
                                           const char* proc_name) {
  DCHECK(IsExtensionEnabled(extension_name));
  return LoadOpenGLFunction(proc_name);
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
