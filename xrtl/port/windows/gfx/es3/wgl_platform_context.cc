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

#include "xrtl/port/windows/gfx/es3/wgl_platform_context.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "xrtl/base/debugging.h"
#include "xrtl/base/flags.h"
#include "xrtl/base/tracing.h"

DEFINE_bool(gl_debug, true, "Enable OpenGL debug validation layer.");

namespace xrtl {
namespace gfx {
namespace es3 {

namespace {

typedef void*(WINAPI* PFNWGLGETPROCADDRESSPROC)(const char* name);
PFNWGLGETPROCADDRESSPROC xrtl_wglGetProcAddress = nullptr;
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTPROC)(HDC hdc);
PFNWGLCREATECONTEXTPROC xrtl_wglCreateContext = nullptr;
typedef BOOL(WINAPI* PFNWGLDELETECONTEXTPROC)(HGLRC hglrc);
PFNWGLDELETECONTEXTPROC xrtl_wglDeleteContext = nullptr;
typedef HGLRC(WINAPI* PFNWGLGETCURRENTCONTEXTCONTEXTPROC)();
PFNWGLGETCURRENTCONTEXTCONTEXTPROC xrtl_wglGetCurrentContext = nullptr;
typedef BOOL(WINAPI* PFNWGLMAKECURRENTPROC)(HDC hdc, HGLRC hglrc);
PFNWGLMAKECURRENTPROC xrtl_wglMakeCurrent = nullptr;
typedef BOOL(WINAPI* PFNWGLSWAPLAYERBUFFERSPROC)(HDC hdc, int planes);
PFNWGLSWAPLAYERBUFFERSPROC xrtl_wglSwapLayerBuffers = nullptr;

void* LoadOpenGLFunction(const char* proc_name) {
  // Try first with wglGetProcAddress, then fallback to GetProcAddress.
  if (xrtl_wglGetProcAddress) {
    void* proc = xrtl_wglGetProcAddress(proc_name);
    if (proc) {
      return proc;
    }
  }
  static HMODULE opengl32_library = nullptr;
  if (!opengl32_library) {
    opengl32_library = ::LoadLibraryW(L"opengl32.dll");
  }
  if (!opengl32_library) {
    LOG(ERROR) << "Unable to load opengl32.dll";
    return static_cast<void*>(nullptr);
  }
  return reinterpret_cast<void*>(::GetProcAddress(opengl32_library, proc_name));
}

bool InitializeCoreWglFunctions() {
  // Load OpenGL32.dll and get the load function we'll use for GLAD.
  xrtl_wglGetProcAddress = reinterpret_cast<PFNWGLGETPROCADDRESSPROC>(
      LoadOpenGLFunction("wglGetProcAddress"));
  xrtl_wglCreateContext = reinterpret_cast<PFNWGLCREATECONTEXTPROC>(
      LoadOpenGLFunction("wglCreateContext"));
  xrtl_wglDeleteContext = reinterpret_cast<PFNWGLDELETECONTEXTPROC>(
      LoadOpenGLFunction("wglDeleteContext"));
  xrtl_wglGetCurrentContext =
      reinterpret_cast<PFNWGLGETCURRENTCONTEXTCONTEXTPROC>(
          LoadOpenGLFunction("wglGetCurrentContext"));
  xrtl_wglMakeCurrent = reinterpret_cast<PFNWGLMAKECURRENTPROC>(
      LoadOpenGLFunction("wglMakeCurrent"));
  xrtl_wglSwapLayerBuffers = reinterpret_cast<PFNWGLSWAPLAYERBUFFERSPROC>(
      LoadOpenGLFunction("wglSwapLayerBuffers"));
  return xrtl_wglGetProcAddress && xrtl_wglCreateContext &&
         xrtl_wglDeleteContext && xrtl_wglGetCurrentContext &&
         xrtl_wglMakeCurrent && xrtl_wglSwapLayerBuffers;
}

std::string GetWglErrorName(uint32_t error) { return std::to_string(error); }

std::string GetWglErrorDescription(uint32_t error) {
  char buffer[256];
  if (::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer,
                       count_of(buffer), NULL) == 0) {
    return "UNKNOWN";
  }
  return std::string(buffer);
}

}  // namespace

ref_ptr<ES3PlatformContext> ES3PlatformContext::Create(
    void* native_display, void* native_window,
    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("ES3PlatformContext#Create");

  auto platform_context = make_ref<WGLPlatformContext>();

  if (!platform_context->Initialize(reinterpret_cast<HDC>(native_display),
                                    reinterpret_cast<HWND>(native_window),
                                    std::move(share_group))) {
    LOG(ERROR) << "Unable to initialize the EGL platform context";
    return nullptr;
  }

  return platform_context;
}

WGLPlatformContext::WGLPlatformContext() = default;

WGLPlatformContext::~WGLPlatformContext() {
  WTF_SCOPE0("WGLPlatformContext#dtor");

  // Finish all context operations.
  if (glrc_) {
    if (MakeCurrent()) {
      Finish();
    }
    ClearCurrent();
  }

  if (glrc_) {
    xrtl_wglDeleteContext(glrc_);
    glrc_ = nullptr;
  }
  if (native_display_) {
    if (is_headless_) {
      // Delete the DC we created for temp usage.
      ::DeleteDC(native_display_);
    }
    native_display_ = nullptr;
  }
  if (native_window_) {
    if (is_headless_) {
      // Delete the temp window.
      ::DestroyWindow(native_window_);
    }
    native_window_ = nullptr;
  }
}

HWND WGLPlatformContext::CreateDummyWindow() {
  static const wchar_t* kWindowClassName = L"XrtlWglDummyWindowClass";

  // Ensure we create the window class we use for the window.
  // This should be process-local so we only need to do it once.
  static std::once_flag register_class_flag;
  std::call_once(register_class_flag, []() {
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = ::DefWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = ::GetModuleHandle(nullptr);
    wcex.hIcon = nullptr;
    wcex.hIconSm = nullptr;
    wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = kWindowClassName;
    if (!::RegisterClassExW(&wcex)) {
      LOG(FATAL) << "Unable to register window class";
    }
  });

  // Setup initial window style.
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
  DWORD window_ex_style = WS_EX_APPWINDOW | WS_EX_CONTROLPARENT;

  // Create window.
  HWND hwnd = ::CreateWindowExW(
      window_ex_style, kWindowClassName, L"(xrtl dummy)", window_style,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr,
      nullptr, ::GetModuleHandle(nullptr), this);
  if (!hwnd) {
    DWORD last_error = ::GetLastError();
    LOG(ERROR) << "Unable to create dummy window: "
               << GetWglErrorName(last_error) << ": "
               << GetWglErrorDescription(last_error);
    return nullptr;
  }

  return hwnd;
}

bool WGLPlatformContext::Initialize(HDC native_display, HWND native_window,
                                    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("WGLPlatformContext#Initialize");

  // We always need a HDC/HWND to use GL. If we are offscreen we'll still create
  // one to use (ugh).
  native_display_ = native_display;
  native_window_ = native_window;
  if (!native_display_ || !native_window_) {
    native_window_ = CreateDummyWindow();
    native_display_ = ::GetDC(native_window_);
    is_headless_ = true;
  }
  if (!native_display_ || !native_window_) {
    LOG(ERROR) << "Unable to create Windows DC for GL init";
    return false;
  }

  // Ensure WGL is initialized. May have been done elsewhere.
  if (!InitializeWGL(native_display_)) {
    LOG(ERROR) << "Failed to initialize WGL; cannot create context";
    return false;
  }

  // Grab the share group context, if it exists.
  HGLRC share_context = nullptr;
  if (share_group) {
    share_context = reinterpret_cast<HGLRC>(share_group->native_handle());
  }

  // Setup a pixel format, even for headless contexts.
  // TODO(benvanik): allow swapchain to specify a format?
  PIXELFORMATDESCRIPTOR pfd = {0};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;
  int pixel_format = ChoosePixelFormat(native_display_, &pfd);
  if (!pixel_format) {
    DWORD last_error = ::GetLastError();
    LOG(ERROR) << "ChoosePixelFormat failed: " << GetWglErrorName(last_error)
               << ": " << GetWglErrorDescription(last_error);
    return false;
  }
  if (SetPixelFormat(native_display_, pixel_format, &pfd) == FALSE) {
    DWORD last_error = ::GetLastError();
    LOG(ERROR) << "SetPixelFormat failed: " << GetWglErrorName(last_error)
               << ": " << GetWglErrorDescription(last_error);
    return false;
  }

  // Ensure we have a context held during creation. It either needs to be the
  // context we are going to share resources with or a temporary context.
  ExclusiveLock share_group_lock;
  auto destroy_context = [this](void* glrc) {
    xrtl_wglMakeCurrent(native_display_, nullptr);
    if (glrc) {
      xrtl_wglDeleteContext(reinterpret_cast<HGLRC>(glrc));
    }
  };
  std::unique_ptr<void, decltype(destroy_context)> bootstrap_glrc{
      nullptr, destroy_context};
  if (share_group) {
    share_group_lock = ExclusiveLock(share_group);
    if (!share_group_lock.is_held()) {
      LOG(ERROR) << "Unable to lock share group context";
      return false;
    }
  } else {
    // Create a temporary context. We need this context in order to call the
    // other context creation functions. We do this even if we have a share
    // group as it's possible that we cannot lock it on this thread.
    bootstrap_glrc.reset(xrtl_wglCreateContext(native_display_));
    if (!bootstrap_glrc) {
      DWORD last_error = ::GetLastError();
      LOG(ERROR) << "wglCreateContext for bootstrap failed: "
                 << GetWglErrorName(last_error) << ": "
                 << GetWglErrorDescription(last_error);
      return false;
    }
    if (xrtl_wglMakeCurrent(native_display_,
                            reinterpret_cast<HGLRC>(bootstrap_glrc.get())) ==
        FALSE) {
      DWORD last_error = ::GetLastError();
      LOG(ERROR) << "wglMakeCurrent for bootstrap failed: "
                 << GetWglErrorName(last_error) << ": "
                 << GetWglErrorDescription(last_error);
      return false;
    }
  }

  // Initialize GLAD, which will fetch the required WGL functions.
  if (!gladLoadWGLLoader(xrtl_wglGetProcAddress, native_display_)) {
    LOG(ERROR) << "Failed to load WGL functions";
    return false;
  }

  int context_flags = 0;
  if (FLAGS_gl_debug) {
    context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
  }

  // Check for robustness support. We should always run with this as it will
  // provide better early error detection and ensure we are writing code that
  // doesn't explode on implementations that are robust-by-default.
  is_robust_access_supported_ = GLAD_WGL_ARB_create_context_robustness == 1;
  if (is_robust_access_supported_) {
    context_flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
  }

  // Create the real context.
  int attrib_list[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB,
      4,
      WGL_CONTEXT_MINOR_VERSION_ARB,
      1,
      WGL_CONTEXT_FLAGS_ARB,
      context_flags,
      WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
      is_robust_access_supported_ ? WGL_LOSE_CONTEXT_ON_RESET_ARB : 0,
      0,  // List terminator.
  };
  glrc_ =
      wglCreateContextAttribsARB(native_display_, share_context, attrib_list);
  if (!glrc_) {
    DWORD last_error = ::GetLastError();
    LOG(ERROR) << "wglCreateContextAttribsARB failed: "
               << GetWglErrorName(last_error) << ": "
               << GetWglErrorDescription(last_error);
    return false;
  }

  // Unlock share group.
  share_group_lock.reset();

  // Cleanup the bootstrap context as we no longer need it after we've created
  // our real context.
  bootstrap_glrc.reset();

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
  static std::once_flag load_gles2_flag;
  static std::atomic<bool> loaded_gles2{false};
  std::call_once(load_gles2_flag, []() {
    loaded_gles2 = gladLoadGLES2Loader(LoadOpenGLFunction);
  });
  if (!loaded_gles2) {
    LOG(ERROR) << "Failed to load GL ES dynamic functions";
    return false;
  }

  // Grab GL info.
  static std::once_flag log_gl_flag;
  std::call_once(log_gl_flag, []() {
    LOG(INFO) << "GL initialized successfully:" << std::endl
              << "GL vendor: " << glGetString(GL_VENDOR) << std::endl
              << "GL renderer: " << glGetString(GL_RENDERER) << std::endl
              << "GL version: " << glGetString(GL_VERSION) << std::endl;
  });

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

bool WGLPlatformContext::InitializeWGL(HDC hdc) {
  WTF_SCOPE0("WGLPlatformContext#InitializeWGL");

  // Attempt to load WGL.
  static std::once_flag load_flag;
  static std::atomic<bool> load_result{false};
  std::call_once(load_flag, [this]() {
    // Grab the few imports we need from OpenGL32 that GLAD doesn't pull in.
    if (!InitializeCoreWglFunctions()) {
      LOG(ERROR) << "Failed to initialize core WGL functions";
      load_result = false;
      return;
    }
    load_result = true;
  });

  return load_result;
}

bool WGLPlatformContext::IsCurrent() {
  if (glrc_) {
    return xrtl_wglGetCurrentContext() == glrc_;
  } else {
    return false;
  }
}

bool WGLPlatformContext::MakeCurrent() {
  WTF_SCOPE0("WGLPlatformContext#MakeCurrent");

  DCHECK(native_display_ != nullptr);
  DCHECK(glrc_ != nullptr);

  if (has_lost_context_) {
    // We've already lost our context - nothing to do.
    return false;
  }
  if (is_robust_access_supported_ && glGetGraphicsResetStatus) {
    GLenum reset_status = glGetGraphicsResetStatus();
    if (reset_status != GL_NO_ERROR) {
      // Context was lost (TDR, driver update, etc).
      has_lost_context_ = true;  // Debounce logging.
      LOG(ERROR) << "wglMakeCurrent failed: GL context " << this << " lost ("
                 << (reset_status == GL_GUILTY_CONTEXT_RESET ? "guilty"
                                                             : "innocent")
                 << ")";
      return false;
    }
  }

  if (IsCurrent()) {
    // No-op.
    return true;
  }

  if (xrtl_wglMakeCurrent(native_display_, glrc_) == FALSE) {
    DWORD last_error = ::GetLastError();
    LOG(ERROR) << "wglMakeCurrent failed: " << GetWglErrorName(last_error)
               << ": " << GetWglErrorDescription(last_error);
    return false;
  }

  return true;
}

void WGLPlatformContext::ClearCurrent() {
  WTF_SCOPE0("WGLPlatformContext#ClearCurrent");
  xrtl_wglMakeCurrent(native_display_, nullptr);
}

void WGLPlatformContext::Flush() {
  WTF_SCOPE0("WGLPlatformContext#Flush");
  DCHECK(IsCurrent());
  glFlush();
}

void WGLPlatformContext::Finish() {
  WTF_SCOPE0("WGLPlatformContext#Finish");
  DCHECK(IsCurrent());
  if (glFinish) {
    glFinish();
  }
}

WGLPlatformContext::RecreateSurfaceResult WGLPlatformContext::RecreateSurface(
    Size2D size_hint) {
  WTF_SCOPE0("WGLPlatformContext#RecreateSurface");

  // NOTE: nothing to do here, as on Windows the default framebuffer is
  // automatically resized.

  return RecreateSurfaceResult::kSuccess;
}

Size2D WGLPlatformContext::QuerySize() {
  DCHECK(native_display_ != nullptr);
  DCHECK(glrc_ != nullptr);

  if (is_headless_) {
    // No-op.
    return {0, 0};
  }

  RECT rect;
  if (::GetClientRect(native_window_, &rect) == FALSE) {
    LOG(ERROR) << "Unable to query window size";
    return {0, 0};
  }
  return {rect.right - rect.left, rect.bottom - rect.top};
}

void WGLPlatformContext::SetSwapBehavior(SwapBehavior swap_behavior) {
  switch (swap_behavior) {
    case SwapBehavior::kImmediate:
      wglSwapIntervalEXT(0);
      break;
    case SwapBehavior::kSynchronize:
      wglSwapIntervalEXT(1);
      break;
    case SwapBehavior::kSynchronizeAndTear:
      if (GLAD_WGL_EXT_swap_control_tear) {
        wglSwapIntervalEXT(-1);
      } else {
        wglSwapIntervalEXT(1);
      }
      break;
  }
}

bool WGLPlatformContext::SwapBuffers(
    std::chrono::milliseconds present_time_utc_millis) {
  if (is_headless_) {
    // No-op.
    return true;
  }
  return xrtl_wglSwapLayerBuffers(native_display_, 0x00000001) == TRUE;
}

void* WGLPlatformContext::GetExtensionProc(const char* extension_name,
                                           const char* proc_name) {
  DCHECK(native_display_ != nullptr);
  DCHECK(IsExtensionEnabled(extension_name));
  return LoadOpenGLFunction(proc_name);
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
