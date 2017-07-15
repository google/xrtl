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

#include "xrtl/port/common/gfx/es3/egl_platform_context.h"

#if !defined(XRTL_PLATFORM_WINDOWS)
#include <dlfcn.h>
#endif  // !XRTL_PLATFORM_WINDOWS

#include <algorithm>
#include <utility>

#include "xrtl/base/debugging.h"
#include "xrtl/base/tracing.h"
#include "xrtl/port/common/gfx/es3/egl_strings.h"
#include "xrtl/tools/target_config/target_config.h"

namespace xrtl {
namespace gfx {
namespace es3 {

namespace {

// A cache of initialized EGLDisplays.
//
// On desktop EGL with the nVidia GPU driver after calling eglTerminate()
// eglGetDisplay() is not able to obtain a valid EGLDisplay. This cache helps
// work around that by keeping the EGLDisplay instances around for reuse at the
// limitation of never really being able to uninitialize EGL.
class EGLDisplayCache {
 public:
  EGLDisplayCache() = default;

  ~EGLDisplayCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto pair : displays_) {
      eglTerminate(pair.second);
    }
    displays_.clear();
  }

  // Checks whether the display is obtained and initialized and either returns
  // the existing display or obtains a new one by calling eglGetDisplay and
  // initializing it with eglInitialize.
  // Returns nullptr if either EGL call fails.
  EGLDisplay LookupOrRegisterDisplay(EGLNativeDisplayType native_display) {
    WTF_SCOPE0("EGLDisplayCache#LookupOrRegisterDisplay");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!InitializeEGL()) {
      return nullptr;
    }
    EGLDisplay display = LookupExistingDisplay(native_display);
    if (display) {
      return display;
    }
    return RegisterNewDisplay(native_display);
  }

 private:
  // Initializes the EGL API.
  // Returns false if the EGL/OpenGLES API is not supported.
  bool InitializeEGL() {
    WTF_SCOPE0("EGLDisplayCache#InitializeEGL");

    // Attempt to bind ES. Note that this has likely already been performed
    // somewhere by someone but lets us ensure we have it.
    static std::once_flag bind_flag;
    static std::atomic<bool> bind_result{false};
    std::call_once(bind_flag, []() {
      debugging::LeakCheckDisabler leak_check_disabler;
      if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        EGLint error_code = eglGetError();
        LOG(ERROR) << "eglBindAPI failed: unable to bind ES API error "
                   << GetEglErrorName(error_code) << ": "
                   << GetEglErrorDescription(error_code);
        bind_result = false;
      } else {
        bind_result = true;
      }
    });

    return bind_result;
  }

  // Looks up an existing EGLDisplay for the given native display.
  // Assumes the mutex is held.
  // Returns nullptr if not found.
  EGLDisplay LookupExistingDisplay(EGLNativeDisplayType native_display) {
    for (const auto& pair : displays_) {
      if (pair.first == native_display) {
        return pair.second;
      }
    }
    return nullptr;
  }

  // Registers a new EGLDisplay for the given native display.
  // Assumes the mutex is held.
  // Returns nullptr if the display could not be initialized.
  EGLDisplay RegisterNewDisplay(EGLNativeDisplayType native_display) {
    WTF_SCOPE0("EGLDisplayCache#RegisterNewDisplay");

    // Get the display handle.
    // This may fail if the given display is not connected (X error, etc).
    // Offscreen targets get the default display, which may be nothing.
    EGLDisplay display = eglGetDisplay(native_display);
    if (display == EGL_NO_DISPLAY) {
      EGLint error_code = eglGetError();
      LOG(ERROR) << "eglGetDisplay failed: binding error "
                 << GetEglErrorName(error_code) << ": "
                 << GetEglErrorDescription(error_code);
      return nullptr;
    }

    // Initialize EGL targeting the given display.
    // This may not work if EGL is not available or the display is not
    // configured correctly.
    // If it has already been called then this is a no-op.
    EGLint major = 0;
    EGLint minor = 0;
    {
      debugging::LeakCheckDisabler leak_check_disabler;
      if (!eglInitialize(display, &major, &minor)) {
        EGLint error_code = eglGetError();
        LOG(ERROR) << "eglInitialize failed: display unavailable error "
                   << GetEglErrorName(error_code) << ": "
                   << GetEglErrorDescription(error_code);
        return nullptr;
      }
    }

    // Query EGL to get some useful debug info.
    LOG(INFO) << "EGL initialized successfully:" << std::endl
              << "EGL vendor: " << eglQueryString(display, EGL_VENDOR)
              << std::endl
              << "EGL version: " << eglQueryString(display, EGL_VERSION)
              << std::endl
              << "EGL client APIs: " << eglQueryString(display, EGL_CLIENT_APIS)
              << std::endl
              << "EGL extensions: " << eglQueryString(display, EGL_EXTENSIONS)
              << std::endl;

    // Setup EGL extension symbols.
    gladLoadEGL();

    // Add to cache forever.
    displays_.push_back({native_display, display});

    return display;
  }

  std::mutex mutex_;

  // In practice, we expect a small number of NativeDisplay's used in the same
  // program, so we just model the display library with a linear array.
  std::vector<std::pair<EGLNativeDisplayType, EGLDisplay>> displays_;
};

// Returns a shared DisplayCache initialized upon first request.
EGLDisplayCache* shared_display_cache() {
  static std::once_flag create_flag;
  static EGLDisplayCache* shared_instance;
  std::call_once(create_flag, []() {
    shared_instance = new EGLDisplayCache();
    atexit([]() { delete shared_instance; });
  });
  DCHECK(shared_instance);
  return shared_instance;
}

// Lookup a function within the dynamically loaded GLESv2 DLL.
void* LookupGlesFunction(const char* name) {
#if defined(XRTL_CONFIG_SWIFTSHADER) && defined(XRTL_PLATFORM_WINDOWS)
  static HMODULE libglesv2 = nullptr;
  if (!libglesv2) {
    libglesv2 = ::LoadLibraryW(L"libGLESv2.dll");
  }
  if (!libglesv2) {
    LOG(ERROR) << "Unable to load libGLESv2.dll";
    return static_cast<void*>(nullptr);
  }
  return ::GetProcAddress(libglesv2, name);
#elif defined(XRTL_CONFIG_SWIFTSHADER)
  // This requires the so to be on the path.
  static void* libglesv2 = nullptr;
  if (!libglesv2) {
    libglesv2 = dlopen("libGLESv2.so", RTLD_LOCAL | RTLD_LAZY);
  }
  if (!libglesv2) {
    LOG(ERROR) << "Unable to load libGLESv2.so";
    return static_cast<void*>(nullptr);
  }
  return dlsym(libglesv2, name);
#else
  static void* libglesv2 = nullptr;
  static void* libglesv2_nvidia = nullptr;
  static bool has_checked_nvidia = false;
  if (!libglesv2) {
    libglesv2 = dlopen("libGLESv2.so.2", RTLD_LOCAL | RTLD_LAZY);
  }
  if (!libglesv2) {
    LOG(ERROR) << "Unable to load libGLESv2.so";
    return static_cast<void*>(nullptr);
  }
  if (!has_checked_nvidia && glGetString) {
    // Nvidia doesn't export glDispatchCompute and other 3.1/3.2 functions
    // from libGLESv2.so for some reason. To work around this we directly
    // probe into their libGLESv2_nvidia.so (which... ugh).
    has_checked_nvidia = true;
    if (std::strstr(reinterpret_cast<const char*>(glGetString(GL_VENDOR)),
                    "NVIDIA") != nullptr) {
      libglesv2_nvidia =
          dlopen("libGLESv2_nvidia.so.2", RTLD_LOCAL | RTLD_LAZY);
    }
  }
  void* proc = dlsym(libglesv2, name);
  if (!proc && libglesv2_nvidia) {
    proc = dlsym(libglesv2_nvidia, name);
  }
  return proc;
#endif  // XRTL_CONFIG_SWIFTSHADER
}

}  // namespace

ref_ptr<ES3PlatformContext> ES3PlatformContext::Create(
    void* native_display, void* native_window,
    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("ES3PlatformContext#Create");

  auto platform_context = make_ref<EGLPlatformContext>();

  if (!platform_context->Initialize(
          reinterpret_cast<EGLNativeDisplayType>(
              reinterpret_cast<uintptr_t>(native_display)),
          reinterpret_cast<EGLNativeWindowType>(
              reinterpret_cast<uintptr_t>(native_window)),
          std::move(share_group))) {
    LOG(ERROR) << "Unable to initialize the EGL platform context";
    return nullptr;
  }

  return platform_context;
}

EGLPlatformContext::EGLPlatformContext() = default;

EGLPlatformContext::~EGLPlatformContext() {
  WTF_SCOPE0("EGLPlatformContext#dtor");

  // Finish all context operations.
  FinishOnShutdown();

  if (egl_context_ != EGL_NO_CONTEXT) {
    eglDestroyContext(egl_display_, egl_context_);
    egl_context_ = EGL_NO_CONTEXT;
  }
  if (egl_surface_ != EGL_NO_SURFACE) {
    eglDestroySurface(egl_display_, egl_surface_);
    egl_surface_ = EGL_NO_SURFACE;
  }
}

bool EGLPlatformContext::Initialize(EGLNativeDisplayType native_display,
                                    EGLNativeWindowType native_window,
                                    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("EGLPlatformContext#Initialize");

  // Lookup an existing EGLDisplay or initialize a new one.
  egl_display_ =
      shared_display_cache()->LookupOrRegisterDisplay(EGL_DEFAULT_DISPLAY);
  if (!egl_display_) {
    LOG(ERROR) << "Unable to register an EGL display";
    return false;
  }

  // If the EGL implementation supports surfaceless binding we can avoid
  // creating a surface for offscreen contexts.
  const char* egl_extensions = eglQueryString(egl_display_, EGL_EXTENSIONS);
  supports_surfaceless_context_ =
      std::strstr(egl_extensions, "EGL_KHR_surfaceless_context") != nullptr;
  if (!supports_surfaceless_context_ && !native_window_) {
    LOG(WARNING) << "EGL implementation does not support "
                    "EGL_KHR_surfaceless_context, creating a dummy surface to "
                    "make it happy";
  }
  supports_configless_context_ =
      std::strstr(egl_extensions, "EGL_MESA_configless_context") != nullptr;

  // Initialize config based on our target display type.
  if (native_window || !supports_configless_context_) {
    ConfigRequestFlag config_request_flags = ConfigRequestFlag::kOpenGLES3;
    config_request_flags |= ConfigRequestFlag::kDepthGE16Required;
    config_request_flags |= ConfigRequestFlag::kStencilGE8Required;
    if (native_window) {
      config_request_flags |= ConfigRequestFlag::kWindowSurfaceType;
    }
    EGLConfig egl_config = nullptr;
    if (!ChooseBestConfig(egl_display_, config_request_flags, &egl_config)) {
      // We need some nice params to the render device so we can do things like
      // give valid surface formats/etc.
      // For now, we just fail.
      LOG(ERROR)
          << "No matching EGL configs found; fallbacks not supported yet";
      return false;
    }
    egl_config_ = egl_config;
  } else {
    VLOG(1) << "EGL supports EGL_MESA_configless_context, skipping config "
               "selection for an offscreen context";
    egl_config_ = 0;
  }

  // Perform common initialization (such as extensions and other queries).
  if (!InitializeContext(std::move(share_group))) {
    LOG(ERROR) << "Unable to initialize common EGL support";
    return false;
  }

  native_display_ = native_display;
  native_window_ = native_window;

  return true;
}

bool EGLPlatformContext::InitializeContext(
    ref_ptr<ES3PlatformContext> share_group) {
  WTF_SCOPE0("EGLPlatformContext#InitializeContext");

  // Warn if we aren't native.
  if (egl_config_ != 0) {
    EGLint is_native = EGL_FALSE;
    eglGetConfigAttrib(egl_display_, egl_config_, EGL_NATIVE_RENDERABLE,
                       &is_native);
    if (is_native == EGL_FALSE) {
      LOG(WARNING)
          << "EGL surface not native; it will require conversion each swap";
    }
  }

  // Create our context.
  // We want the best we can get, so run down 3.2, .1, .0.
  EGLContext share_context =
      share_group ? share_group->native_handle() : EGL_NO_CONTEXT;
  EGLContext egl_context = EGL_NO_CONTEXT;
  for (int minor_version = 2; minor_version >= 0; --minor_version) {
    EGLint context_attribs[] = {
        // OpenGL ES version 3.
        EGL_CONTEXT_MAJOR_VERSION_KHR, 3, EGL_CONTEXT_MINOR_VERSION_KHR,
        minor_version,
        // End of list.
        EGL_NONE,
    };
    egl_context = eglCreateContext(egl_display_, egl_config_, share_context,
                                   context_attribs);
    if (egl_context != EGL_NO_CONTEXT) {
      // Got one!
      break;
    }
  }
  if (egl_context == EGL_NO_CONTEXT) {
    EGLint error_code = eglGetError();
    LOG(ERROR) << "eglCreateContext failed: unable to create context, "
               << GetEglErrorName(error_code) << ": "
               << GetEglErrorDescription(error_code);
    return false;
  }
  egl_context_ = egl_context;

  // Initialize the target surface (if not offscreen).
  // We must create a dummy surface before we try to make the context current.
  if (native_window_ || !supports_surfaceless_context_) {
    if (RecreateSurface({0, 0}) != RecreateSurfaceResult::kSuccess) {
      LOG(ERROR) << "Unable to create window surface";
      return false;
    }
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
  static std::once_flag load_gles2_flag;
  static std::atomic<bool> loaded_gles2{false};
  std::call_once(load_gles2_flag, []() {
    loaded_gles2 = gladLoadGLES2Loader(LookupGlesFunction);
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
              << "GL version: " << glGetString(GL_VERSION) << std::endl
              << "GL extensions: " << glGetString(GL_EXTENSIONS) << std::endl;
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

  return true;
}

bool EGLPlatformContext::ChooseBestConfig(
    EGLDisplay egl_display, ConfigRequestFlag config_request_flags,
    EGLConfig* out_egl_config) {
  *out_egl_config = nullptr;

  // NOTE: we don't use eglChooseConfig here as it's totally broken on Android.

  // Query config count to preallocate result buffer.
  EGLint config_count = 0;
  if (!eglGetConfigs(egl_display, nullptr, 0, &config_count)) {
    EGLint error_code = eglGetError();
    LOG(ERROR) << "eglGetConfigs failed: unable to query config count, "
               << GetEglErrorName(error_code) << ": "
               << GetEglErrorDescription(error_code);
    return false;
  }
  if (!config_count) {
    LOG(ERROR) << "eglGetConfigs failed: no EGL configs found (at all)";
    return false;
  }

  // Get all configs.
  std::vector<EGLConfig> all_configs(config_count);
  if (!eglGetConfigs(egl_display, all_configs.data(),
                     static_cast<EGLint>(all_configs.size()), &config_count)) {
    EGLint error_code = eglGetError();
    LOG(ERROR) << "eglGetConfigs failed: unable to query all configs, "
               << GetEglErrorName(error_code) << ": "
               << GetEglErrorDescription(error_code);
    return false;
  }

  // Build a list of configs that meet our min-spec.
  std::vector<EGLConfig> min_spec_configs;
  for (EGLConfig egl_config : all_configs) {
    if (ValidateConfig(egl_display, egl_config, config_request_flags)) {
      // Passed our min-bar.
      min_spec_configs.push_back(egl_config);
    }
  }
  if (min_spec_configs.empty()) {
    LOG(ERROR) << "Found no EGL configs out of " << all_configs.size()
               << " that meet our minimum specifications";
    return false;
  }
  VLOG(1) << "Found " << min_spec_configs.size() << " EGL configs out of "
          << all_configs.size()
          << " that meet our minimum specification, selecting the best";

  // Sort by buffer size, performance, etc.
  SortConfigs(egl_display, &min_spec_configs, config_request_flags);

  // Take the best (first).
  EGLConfig best_config = min_spec_configs[0];
  DumpConfig(egl_display, best_config);

  *out_egl_config = best_config;
  return true;
}

// Toggle on tons of config selection debug logging.
// Useful if no matching configs are found and you want to know why.
// #define LOG_CONFIG_BAIL_OUT(...)
#define LOG_CONFIG_BAIL_OUT() VLOG(3)
#define LOG_CONFIG_MISMATCH_BAIL_OUT(attr) \
  LOG_CONFIG_BAIL_OUT() << "  Skipped: mismatch " #attr " "
#define LOG_CONFIG_GET_BAIL_OUT(attr) \
  LOG_CONFIG_BAIL_OUT() << "  Skipped: eglGetConfigAttrib failed for " #attr

bool EGLPlatformContext::ValidateConfig(
    EGLDisplay egl_display, EGLConfig egl_config,
    ConfigRequestFlag config_request_flags) {
  LOG_CONFIG_BAIL_OUT() << "Testing config " << egl_config
                        << " for min-spec...";

  // Ensure the config supports our target format (window or offscreen).
  EGLint surface_type = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_SURFACE_TYPE,
                          &surface_type)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_SURFACE_TYPE);
    return false;
  }
  if ((config_request_flags & ConfigRequestFlag::kPBufferSurfaceType) ==
      ConfigRequestFlag::kPBufferSurfaceType) {
    if ((surface_type & EGL_PBUFFER_BIT) != EGL_PBUFFER_BIT) {
      // Not the right surface type.
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_SURFACE_TYPE)
          << "need to draw to pbuffer";
      return false;
    }
  }
  if ((config_request_flags & ConfigRequestFlag::kWindowSurfaceType) ==
      ConfigRequestFlag::kWindowSurfaceType) {
    if ((surface_type & EGL_WINDOW_BIT) != EGL_WINDOW_BIT) {
      // Not the right surface type.
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_SURFACE_TYPE)
          << "need to draw to window";
      return false;
    }
  }

  // Ensure ES 2 is supported.
  // TODO(benvanik): check for ES 3 to see if we can use that.
  EGLint renderable_type = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_RENDERABLE_TYPE,
                          &renderable_type)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_RENDERABLE_TYPE);
    return false;
  }
  if ((config_request_flags & ConfigRequestFlag::kOpenGLES2) ==
      ConfigRequestFlag::kOpenGLES2) {
    if ((renderable_type & EGL_OPENGL_ES2_BIT) != EGL_OPENGL_ES2_BIT) {
      // No support for ES 2. Likely an ES1 context.
      // AFAIK every 3 context also supports 2, so it's OK to bail here.
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_RENDERABLE_TYPE) << "need GL ES 2";
      return false;
    }
  }
  if ((config_request_flags & ConfigRequestFlag::kOpenGLES3) ==
      ConfigRequestFlag::kOpenGLES3) {
    if ((renderable_type & EGL_OPENGL_ES3_BIT) != EGL_OPENGL_ES3_BIT) {
      // If we wanted just an ES 3 context bail out, otherwise we'll let both
      // 2 and 3 go through to the sort (which will prefer 3).
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_RENDERABLE_TYPE) << "need GL ES 3";
      return false;
    }
  }

  // Ensure the ES 2 context is conformant with the spec. Apparently some
  // devices may have non-conformant configs that do weird things.
  EGLint conformant = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_CONFORMANT,
                          &conformant)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_CONFORMANT);
    return false;
  }
  if ((conformant & EGL_OPENGL_ES2_BIT) != EGL_OPENGL_ES2_BIT) {
    // May still be usable, but I don't trust it.
    LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_CONFORMANT) << "need GL ES 2 conformance";
    return false;
  }

  // See if this is a 'slow' config (according to the OS). It may not always
  // be slow.
  EGLint config_caveat = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_CONFIG_CAVEAT,
                          &config_caveat)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_CONFIG_CAVEAT);
    return false;
  }
  if ((config_caveat & EGL_SLOW_CONFIG) == EGL_SLOW_CONFIG) {
    // NOTE: we allow slow configs here but we always sort them last because
    //       certain Android platforms will be silly and mark all configs as
    //       slow >_>
  }

  // Ignore layers (used for overlays and such).
  EGLint level = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_LEVEL, &level)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_LEVEL);
    return false;
  }
  if (level != 0) {
    // Overlay layer.
    LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_LEVEL) << "only want layer 0";
    return false;
  }

  // Opaque views *should* be faster (less compositing work).
  EGLint transparent_type = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_TRANSPARENT_TYPE,
                          &transparent_type)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_TRANSPARENT_TYPE);
    return false;
  }
  if (transparent_type != EGL_NONE) {
    // We could allow transparent configs but rank them lower, as they are
    // likely more expensive to composite.
    LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_TRANSPARENT_TYPE) << "wanted opaque";
    return false;
  }

  // Require 8-bits per color channel. We don't care about alpha.
  EGLint red_size = 0, green_size = 0, blue_size = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_RED_SIZE, &red_size) ||
      !eglGetConfigAttrib(egl_display, egl_config, EGL_GREEN_SIZE,
                          &green_size) ||
      !eglGetConfigAttrib(egl_display, egl_config, EGL_BLUE_SIZE, &blue_size)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_RED_SIZE);
    return false;
  }
  if (red_size != 8 || green_size != 8 || blue_size != 8) {
    // Probably RGB565, luminance, or depth-only.
    LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_RED_SIZE)
        << "need RGB 888, have " << red_size << green_size << blue_size;
    return false;
  }

  // Ensure we have an alpha channel if we asked for it.
  EGLint alpha_size = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_ALPHA_SIZE,
                          &alpha_size)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_ALPHA_SIZE);
    return false;
  }
  if ((config_request_flags & ConfigRequestFlag::kAlpha8Required) ==
      ConfigRequestFlag::kAlpha8Required) {
    if (alpha_size != 8) {
      // No alpha channel or a small one (I've seen 1-bit alpha).
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_ALPHA_SIZE) << "8bpp alpha required";
      return false;
    }
  } else {
#if defined(XRTL_PLATFORM_ANDROID)
    // On Android we want 32bit surfaces if possible (as RGBX is faster than
    // RGB), however on X11 32bit surfaces don't work even though it lists
    // them so we have to blacklist them all.
    if (alpha_size) {
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_ALPHA_SIZE) << "no alpha required";
      return false;
    }
#endif  // XRTL_PLATFORM_ANDROID
  }

  // Verify depth/stencil buffers.
  EGLint depth_size = 0, stencil_size = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_DEPTH_SIZE,
                          &depth_size) ||
      !eglGetConfigAttrib(egl_display, egl_config, EGL_STENCIL_SIZE,
                          &stencil_size)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_DEPTH_SIZE);
    return false;
  }
  if ((depth_size || stencil_size) && ((depth_size + stencil_size) != 16 &&
                                       (depth_size + stencil_size) != 32)) {
    // Ignore non-word depths - they are never fast.
    LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_DEPTH_SIZE)
        << "depth (" << depth_size << ") + stencil (" << stencil_size
        << ") not word aligned";
    return false;
  }
  if ((config_request_flags & ConfigRequestFlag::kDepthGE16Required) ==
      ConfigRequestFlag::kDepthGE16Required) {
    if (depth_size < 16) {
      // No depth. We need at least 16. We'll sort by higher precision later.
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_DEPTH_SIZE)
          << ">=16bpp depth required, had " << depth_size;
      return false;
    }
  } else {
    // NOTE: we allow non-zero depth channels (as we won't create or use them)
    //       though we want to sort 24+8 higher (as 16 can sometimes be slow).
  }
  if ((config_request_flags & ConfigRequestFlag::kStencilGE8Required) ==
      ConfigRequestFlag::kStencilGE8Required) {
    if (stencil_size < 8) {
      // No stencil. We need at least 8. We'll sort by higher precision later.
      LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_STENCIL_SIZE)
          << ">=8bpp stencil required, had " << stencil_size;
      return false;
    }
  } else {
    // NOTE: we allow non-zero stencil channels (as we won't create or use them)
  }

  // No multisampling (probably).
  EGLint samples = 0;
  EGLint sample_buffers = 0;
  if (!eglGetConfigAttrib(egl_display, egl_config, EGL_SAMPLES, &samples) ||
      !eglGetConfigAttrib(egl_display, egl_config, EGL_SAMPLE_BUFFERS,
                          &sample_buffers)) {
    LOG_CONFIG_GET_BAIL_OUT(EGL_SAMPLES);
    return false;
  }
  if (samples != 0 || sample_buffers != 0) {
    // NOTE: usable, but not ideal.
    LOG_CONFIG_MISMATCH_BAIL_OUT(EGL_SAMPLES) << "don't want multisampling";
    return false;
  }

  return true;
}

void EGLPlatformContext::SortConfigs(EGLDisplay egl_display,
                                     std::vector<EGLConfig>* egl_configs,
                                     ConfigRequestFlag config_request_flags) {
  // Comparison returns true if a should come before b.
  std::sort(
      egl_configs->begin(), egl_configs->end(),
      [egl_display, config_request_flags](EGLConfig egl_config_a,
                                          EGLConfig egl_config_b) {
        // Prefer OpenGL ES 3 over 2.
        EGLint renderable_type_a = 0, renderable_type_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_RENDERABLE_TYPE,
                           &renderable_type_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_RENDERABLE_TYPE,
                           &renderable_type_b);
        bool has_es_3_a =
            (renderable_type_a & EGL_OPENGL_ES3_BIT) == EGL_OPENGL_ES3_BIT;
        bool has_es_3_b =
            (renderable_type_b & EGL_OPENGL_ES3_BIT) == EGL_OPENGL_ES3_BIT;
        if (has_es_3_a != has_es_3_b) {
          return has_es_3_a;
        }

        // Prefer configs that are native renderable - it generally means they
        // have real system compositor support and will be better.
        EGLint native_renderable_a = 0, native_renderable_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_NATIVE_RENDERABLE,
                           &native_renderable_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_NATIVE_RENDERABLE,
                           &native_renderable_b);
        if (native_renderable_a != native_renderable_b) {
          return native_renderable_a == EGL_TRUE;
        }

        // Prefer opaque configs.
        EGLint transparent_type_a = 0, transparent_type_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_TRANSPARENT_TYPE,
                           &transparent_type_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_TRANSPARENT_TYPE,
                           &transparent_type_b);
        if (transparent_type_a != transparent_type_b) {
          return transparent_type_a == EGL_NONE;
        }

        // Prefer configs not deemed slow by moving slow ones last.
        EGLint config_caveat_a = 0, config_caveat_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_CONFIG_CAVEAT,
                           &config_caveat_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_CONFIG_CAVEAT,
                           &config_caveat_b);
        bool slow_a = (config_caveat_a & EGL_SLOW_CONFIG) == EGL_SLOW_CONFIG;
        bool slow_b = (config_caveat_b & EGL_SLOW_CONFIG) == EGL_SLOW_CONFIG;
        if (slow_a != slow_b) {
          return slow_a ? false : true;
        }

        // Prefer larger alpha bit depths.
        EGLint alpha_size_a = 0, alpha_size_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_ALPHA_SIZE,
                           &alpha_size_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_ALPHA_SIZE,
                           &alpha_size_b);
        if (alpha_size_a != alpha_size_b) {
          return alpha_size_a > alpha_size_b;
        }

        // Prefer larger depth buffer bit depths if we wanted a depth buffer,
        // and otherwise smaller bit depths (hopefully zero).
        EGLint depth_size_a = 0, depth_size_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_DEPTH_SIZE,
                           &depth_size_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_DEPTH_SIZE,
                           &depth_size_b);
        if (depth_size_a != depth_size_b) {
          if ((config_request_flags & ConfigRequestFlag::kDepthGE16Required) ==
              ConfigRequestFlag::kDepthGE16Required) {
            // Largest depth bits.
            return depth_size_a > depth_size_b;
          } else {
            // Smallest depth bits.
            return depth_size_a < depth_size_b;
          }
        }

        // Prefer larger stencil buffer bit stencils if we wanted a stencil
        // buffer, and otherwise smaller bit stencils (hopefully zero).
        EGLint stencil_size_a = 0, stencil_size_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_STENCIL_SIZE,
                           &stencil_size_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_STENCIL_SIZE,
                           &stencil_size_b);
        if (stencil_size_a != stencil_size_b) {
          if ((config_request_flags & ConfigRequestFlag::kStencilGE8Required) ==
              ConfigRequestFlag::kStencilGE8Required) {
            // Largest stencil bits.
            return stencil_size_a > stencil_size_b;
          } else {
            // Smallest stencil bits.
            return stencil_size_a < stencil_size_b;
          }
        }

        // Fallback to relying on config id to sort.
        EGLint config_id_a = 0, config_id_b = 0;
        eglGetConfigAttrib(egl_display, egl_config_a, EGL_CONFIG_ID,
                           &config_id_a);
        eglGetConfigAttrib(egl_display, egl_config_b, EGL_CONFIG_ID,
                           &config_id_b);
        return config_id_a < config_id_b;
      });
}

void EGLPlatformContext::DumpConfig(EGLDisplay egl_display,
                                    EGLConfig egl_config) {
  EGLint v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_CONFIG_ID, &v);
  VLOG(1) << "  EGL_CONFIG_ID = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_SURFACE_TYPE, &v);
  VLOG(1) << "  EGL_SURFACE_TYPE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_RENDERABLE_TYPE, &v);
  VLOG(1) << "  EGL_RENDERABLE_TYPE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_RENDERABLE, &v);
  VLOG(1) << "  EGL_NATIVE_RENDERABLE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_CONFORMANT, &v);
  VLOG(1) << "  EGL_CONFORMANT = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_CONFIG_CAVEAT, &v);
  VLOG(1) << "  EGL_CONFIG_CAVEAT = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_COLOR_BUFFER_TYPE, &v);
  VLOG(1) << "  EGL_COLOR_BUFFER_TYPE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_LEVEL, &v);
  VLOG(1) << "  EGL_LEVEL = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_TRANSPARENT_TYPE, &v);
  VLOG(1) << "  EGL_TRANSPARENT_TYPE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_RED_SIZE, &v);
  VLOG(1) << "  EGL_RED_SIZE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_GREEN_SIZE, &v);
  VLOG(1) << "  EGL_GREEN_SIZE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_BLUE_SIZE, &v);
  VLOG(1) << "  EGL_BLUE_SIZE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_ALPHA_SIZE, &v);
  VLOG(1) << "  EGL_ALPHA_SIZE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_DEPTH_SIZE, &v);
  VLOG(1) << "  EGL_DEPTH_SIZE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_STENCIL_SIZE, &v);
  VLOG(1) << "  EGL_STENCIL_SIZE = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_SAMPLES, &v);
  VLOG(1) << "  EGL_SAMPLES = " << v;
  eglGetConfigAttrib(egl_display, egl_config, EGL_SAMPLE_BUFFERS, &v);
  VLOG(1) << "  EGL_SAMPLE_BUFFERS = " << v;
}

bool EGLPlatformContext::IsCurrent() {
  if (egl_context_ != EGL_NO_CONTEXT) {
    return eglGetCurrentContext() == egl_context_;
  } else {
    return false;
  }
}

bool EGLPlatformContext::MakeCurrent() {
  WTF_SCOPE0("EGLPlatformContext#MakeCurrent");

  DCHECK_NE(egl_display_, EGL_NO_DISPLAY);
  DCHECK_NE(egl_context_, EGL_NO_CONTEXT);

  if (IsCurrent()) {
    // No-op.
    return true;
  }

  debugging::LeakCheckDisabler leak_check_disabler;
  if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
    EGLint error_code = eglGetError();
    if (error_code == EGL_CONTEXT_LOST) {
      // TODO(benvanik): context loss handling. Fire event?
      return false;
    } else {
      LOG(ERROR) << "eglMakeCurrent failed: binding error "
                 << GetEglErrorName(error_code) << ": "
                 << GetEglErrorDescription(error_code);
      return false;
    }
  }

  return true;
}

void EGLPlatformContext::ClearCurrent() {
  WTF_SCOPE0("EGLPlatformContext#ClearCurrent");
  if (egl_display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
  }
}

void EGLPlatformContext::Flush() {
  WTF_SCOPE0("EGLPlatformContext#Flush");
  DCHECK_NE(egl_context_, EGL_NO_CONTEXT);
  DCHECK(IsCurrent());
  glFlush();
}

void EGLPlatformContext::Finish() {
  WTF_SCOPE0("EGLPlatformContext#Finish");
  DCHECK_NE(egl_context_, EGL_NO_CONTEXT);
  DCHECK(IsCurrent());
  glFinish();
}

void EGLPlatformContext::FinishOnShutdown() {
  WTF_SCOPE0("EGLPlatformContext#FinishOnShutdown");
  if (egl_context_ == EGL_NO_CONTEXT) {
    return;
  }

  if (egl_context_ != eglGetCurrentContext()) {
    debugging::LeakCheckDisabler leak_check_disabler;
    if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_,
                        egl_context_)) {
      EGLint error_code = eglGetError();
      LOG(WARNING) << "eglMakeCurrent on shutdown failed: binding error "
                   << GetEglErrorName(error_code) << ": "
                   << GetEglErrorDescription(error_code);
      return;
    }
  }

  glFinish();

  if (egl_display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
  }
}

EGLPlatformContext::RecreateSurfaceResult EGLPlatformContext::RecreateSurface(
    Size2D size_hint) {
  WTF_SCOPE0("EGLPlatformContext#RecreateSurface");

  DCHECK_NE(egl_display_, EGL_NO_DISPLAY);
  DCHECK_NE(egl_context_, EGL_NO_CONTEXT);

  // The EGL standard says creating a new window surface when there is an
  // existing one will lead to EGL_BAD_ALLOC error, so we destroy the current
  // window surface first. This can cause broken rendering if we fail to create
  // the new one, but that would have probably happened anyway.
  if (egl_surface_ != EGL_NO_SURFACE) {
    // To ensure the current window surface gets destroyed, we first detach it.
#if !defined(XRTL_CONFIG_SWIFTSHADER)  // Swiftshader bug.
    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context_);
#endif  // !XRTL_CONFIG_SWIFTSHADER
    eglDestroySurface(egl_display_, egl_surface_);
    egl_surface_ = EGL_NO_SURFACE;
  }

  if (native_window_) {
    // Create a display surface with the given config.
    // Grab the format of the config. We'll use this to set the window
    // buffer geometry.
    EGLint format;
    if (!eglGetConfigAttrib(egl_display_, egl_config_, EGL_NATIVE_VISUAL_ID,
                            &format)) {
      EGLint error_code = eglGetError();
      LOG(ERROR)
          << "eglGetConfigAttrib failed: unable to get native format; error "
          << GetEglErrorName(error_code) << ": "
          << GetEglErrorDescription(error_code);
      return RecreateSurfaceResult::kInvalidTarget;
    }

#if defined(XRTL_PLATFORM_ANDROID)
    // Set window to use the format of the config.
    ANativeWindow_setBuffersGeometry(
        static_cast<ANativeWindow*>(native_window_), 0, 0, format);
#endif  // XRTL_PLATFORM_ANDROID

    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config_,
                                          native_window_, nullptr);
  } else {
    if (supports_surfaceless_context_) {
      // No-op, as we don't need a surface.
      return RecreateSurfaceResult::kSuccess;
    }

    // Create a Pbuffer just to make EGL happy.
    EGLint pbuffer_attribs[] = {
        EGL_WIDTH,  16,  // Pbuffer width, in pixels.
        EGL_HEIGHT, 16,  // Pbuffer height, in pixels.
        EGL_NONE,        // End of list.
    };
    egl_surface_ =
        eglCreatePbufferSurface(egl_display_, egl_config_, pbuffer_attribs);
  }
  if (egl_surface_ == EGL_NO_SURFACE) {
    EGLint error_code = eglGetError();
    LOG(ERROR) << (native_window_ ? "eglCreateWindowSurface"
                                  : "eglCreatePbufferSurface")
               << " failed: unable to create surface; error "
               << GetEglErrorName(error_code) << ": "
               << GetEglErrorDescription(error_code);
    return RecreateSurfaceResult::kOutOfMemory;
  }

  // The default behavior is implementation defined, though Android mostly uses
  // DESTROYED. We force it here so we know we are running the same on all
  // platforms.
  // https://www.khronos.org/registry/egl/sdk/docs/man/html/eglSurfaceAttrib.xhtml
  eglSurfaceAttrib(egl_display_, egl_surface_, EGL_SWAP_BEHAVIOR,
                   EGL_BUFFER_DESTROYED);

  // Bind the surface for use. This may fail even if the surface was created
  // successfully.
  if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
    EGLint error_code = eglGetError();
    LOG(ERROR) << "eglMakeCurrent failed: unable to make current after "
                  "creating surface; error "
               << GetEglErrorName(error_code) << ": "
               << GetEglErrorDescription(error_code);
    return RecreateSurfaceResult::kDeviceLost;
  }

  return RecreateSurfaceResult::kSuccess;
}

Size2D EGLPlatformContext::QuerySize() {
  DCHECK_NE(egl_display_, EGL_NO_DISPLAY);
  DCHECK_NE(egl_context_, EGL_NO_CONTEXT);

  if (!native_window_) {
    // No-op.
    return {0, 0};
  }

  int real_width;
  int real_height;
  if (!eglQuerySurface(egl_display_, egl_surface_, EGL_WIDTH, &real_width) ||
      !eglQuerySurface(egl_display_, egl_surface_, EGL_HEIGHT, &real_height)) {
    EGLint error_code = eglGetError();
    LOG(ERROR)
        << "eglQuerySurface failed: unable to get surface dimensions; error "
        << GetEglErrorName(error_code) << ": "
        << GetEglErrorDescription(error_code);
    return {0, 0};
  }

  return {real_width, real_height};
}

void EGLPlatformContext::SetSwapBehavior(SwapBehavior swap_behavior) {
  switch (swap_behavior) {
    case SwapBehavior::kImmediate:
      eglSwapInterval(egl_display_, 0);
      break;
    case SwapBehavior::kSynchronize:
      eglSwapInterval(egl_display_, 1);
      break;
    case SwapBehavior::kSynchronizeAndTear:
      // TODO(benvanik): try to use glXSwapIntervalEXT if on linux.
      eglSwapInterval(egl_display_, 1);
      break;
  }
}

bool EGLPlatformContext::SwapBuffers(
    std::chrono::milliseconds present_time_utc_millis) {
  if (!native_window_) {
    // No-op.
    return true;
  }

  // TODO(benvanik): use EGL_ANDROID_presentation_time. We need to make the call
  //                 to eglPresentationTimeANDROID before eglSwapBuffers.

  return eglSwapBuffers(egl_display_, egl_surface_) == EGL_TRUE;
}

void* EGLPlatformContext::GetExtensionProc(const char* extension_name,
                                           const char* proc_name) {
  DCHECK_NE(egl_display_, EGL_NO_DISPLAY);
  DCHECK(IsExtensionEnabled(extension_name));
  return reinterpret_cast<void*>(eglGetProcAddress(proc_name));
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
