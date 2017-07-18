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

#ifndef XRTL_GFX_ES3_ES3_PLATFORM_CONTEXT_H_
#define XRTL_GFX_ES3_ES3_PLATFORM_CONTEXT_H_

#include <mutex>
#include <utility>

#include "xrtl/base/geometry.h"
#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/es3/es3_common.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// Wraps a native GL context.
// GL contexts are bound to a thread with MakeCurrent and may only be used from
// that thread until it is rebound to another. Rebinding can be extremely
// expensive (requiring a full GPU flush and possibly a finish) and should be
// avoided if possible.
//
// The cost of a GL context varies per-platform, with some being very cheap
// (as they can be created without backbuffers when not required), while
// others require default backbuffers and other associated stuff. Using multiple
// contexts simultaneously is not free-scaling as many implementations take
// global locks.
//
// To more safely manage the platform context use Lock to get a RAII wrapper
// that ensures the context is held exclusively by the caller until the scope
// ends.
class ES3PlatformContext : public RefObject<ES3PlatformContext> {
 public:
  // RAII usage lock for use of a context on the current thread.
  // No other thread will be able to use the context until the Lock is
  // reset. If this is called repeatedly from the same thread the cost will be
  // low however performance will degrade rapidly if multiple threads are
  // attempting to acquire the lock.
  //
  // It's possible for a lock to fail if the context cannot be acquired (such
  // as when it is lost/destroyed). Use is_held to check this before attempting
  // to use the context.
  //
  // The context will remain bound to the current thread after the lock has
  // yielded.
  //
  // Usage:
  //  {
  //    // Acquire lock for this scope.
  //    ES3PlatformContext::ThreadLock context_lock(context);
  //    DCHECK(context_lock.is_held());
  //    // Use context here.
  //  }
  //  // NOTE: context may still be bound here, but don't trust it.
  class ThreadLock {
   public:
    ThreadLock() = default;
    ThreadLock(ThreadLock&& other) {  // NOLINT
      std::swap(context_, other.context_);
      lock_ = std::move(other.lock_);
    }
    explicit ThreadLock(ES3PlatformContext* context)
        : ThreadLock(ref_ptr<ES3PlatformContext>(context)) {}
    explicit ThreadLock(ref_ptr<ES3PlatformContext> context) {
      reset(std::move(context));
    }
    ~ThreadLock() { reset(); }

    // Returns true if the context thread lock is held.
    bool is_held() const { return context_ != nullptr; }

    // Unlocks the context lock.
    void reset() { reset(nullptr); }

    // Locks the given context, unlocking the previous context (if any).
    void reset(ref_ptr<ES3PlatformContext> context) {
      if (context == context_) {
        return;
      }
      if (context_) {
        context_->Unlock(std::move(lock_));
        context_.reset();
      }
      context_ = std::move(context);
      if (context_ && !context_->Lock(false, &lock_)) {
        context_ = nullptr;
      }
    }

   private:
    ref_ptr<ES3PlatformContext> context_;
    std::unique_lock<std::recursive_mutex> lock_;
  };

  // RAII usage lock for use of a context on the current thread.
  // No other thread will be able to use the context until the lock is
  // reset. Once the lock is released the context will be cleared from the
  // current thread, meaning that back-to-back locking of the same context will
  // incur the same costs as locking any other context.
  //
  // It's possible for a lock to fail if the context cannot be acquired (such
  // as when it is lost/destroyed). Use is_held to check this before attempting
  // to use the context.
  //
  // Usage:
  //  {
  //    // Acquire lock for this scope.
  //    ES3PlatformContext::ExclusiveLock context_lock(context);
  //    DCHECK(context_lock.is_held());
  //    // Use context here.
  //  }
  //  // NOTE: context will not be bound here.
  class ExclusiveLock {
   public:
    ExclusiveLock() = default;
    ExclusiveLock(ExclusiveLock&& other) {  // NOLINT
      std::swap(context_, other.context_);
      lock_ = std::move(other.lock_);
    }
    explicit ExclusiveLock(ES3PlatformContext* context)
        : ExclusiveLock(ref_ptr<ES3PlatformContext>(context)) {}
    explicit ExclusiveLock(ref_ptr<ES3PlatformContext> context) {
      reset(std::move(context));
    }
    ~ExclusiveLock() { reset(); }
    ExclusiveLock& operator=(ExclusiveLock& other) {  // NOLINT
      context_ = std::move(other.context_);
      lock_ = std::move(other.lock_);
      return *this;
    }

    // Returns true if the context thread lock is held.
    bool is_held() const { return context_ != nullptr; }

    // Unlocks the context lock.
    void reset() { reset(nullptr); }

    // Locks the given context, unlocking the previous context (if any).
    void reset(ref_ptr<ES3PlatformContext> context) {
      if (context == context_) {
        return;
      }
      if (context_) {
        context_->Unlock(std::move(lock_));
        context_.reset();
      }
      context_ = std::move(context);
      if (context_ && !context_->Lock(true, &lock_)) {
        context_ = nullptr;
      }
    }

   private:
    ref_ptr<ES3PlatformContext> context_;
    std::unique_lock<std::recursive_mutex> lock_;
  };

  // Creates a new platform context.
  // If a display and window is provided the context will be created compatible
  // with that pair and otherwise it will be created for offscreen use.
  static ref_ptr<ES3PlatformContext> Create(
      void* native_display, void* native_window,
      ref_ptr<ES3PlatformContext> share_group);
  static ref_ptr<ES3PlatformContext> Create(void* native_display,
                                            void* native_window) {
    return Create(native_display, native_window, {});
  }
  static ref_ptr<ES3PlatformContext> Create(
      ref_ptr<ES3PlatformContext> share_group) {
    return Create(nullptr, nullptr, std::move(share_group));
  }
  static ref_ptr<ES3PlatformContext> Create() {
    return Create(nullptr, nullptr, {});
  }

  // Acquires a thread-locked context that is in a share-group with the given
  // context. Future calls to AcquireThreadContext from the same thread will
  // return the same context. It's safe to use this context with a ThreadLock
  // and keep it current on the calling thread. Avoid using thread-locked
  // contexts with ExclusiveLock as it can cause undefined behavior.
  //
  // The context will be retained for the life of the calling thread or until
  // ReleaseThreadContext is called.
  //
  // Returns the thread-locked context, if one could be created. If the given
  // context happens to already be the thread-locked context for the current
  // thread that will be returned.
  static ref_ptr<ES3PlatformContext> AcquireThreadContext(
      ref_ptr<ES3PlatformContext> existing_context);

  // Releases a thread-locked context for the current thread, if any exists.
  // Calling AcquireThreadContext will recreate a new context for the thread.
  static void ReleaseThreadContext();

  // Acquires a context to use for short operations.
  // If a context is already made current on the calling thread this will return
  // that context. Otherwise, this will return a thread-locked context as if
  // AcquireThreadContext had been used.
  static ThreadLock LockTransientContext(
      ref_ptr<ES3PlatformContext> existing_context);

  virtual ~ES3PlatformContext();

  // Native platform context handle (such as EGLContext).
  virtual void* native_handle() const = 0;

  // Checks whether this render device is attached to the current thread.
  virtual bool IsCurrent() = 0;
  // Binds the context to the current thread.
  // Only one thread may interact with the GL device at any given time.
  // To ensure sanity, try to only ever interact with the GL device from one
  // thread.
  // Returns false when the context fails to bind. No GL calls should
  // be made if the device cannot be made current.
  virtual bool MakeCurrent() = 0;
  // Clears the context from the current thread.
  virtual void ClearCurrent() = 0;

  // Flushes the context without synchronizing with the GPU.
  virtual void Flush() = 0;
  // Flushes the context and waits for the GPU to complete all of the commands.
  virtual void Finish() = 0;

  // Checks whether the given extension is supported.
  // Must include the GL_ prefix.
  bool IsExtensionSupported(const char* extension_name);
  // Checks whether the given extension has been enabled.
  bool IsExtensionEnabled(const char* extension_name);
  // Enables an extension for use, if it is supported.
  bool EnableExtension(const char* extension_name);
  // Tries to enable the given extension.
  // Platforms that do not require enabling are a no-op.
  virtual bool TryEnableExtension(const char* extension_name) {
    return IsExtensionSupported(extension_name);
  }
  // Gets the extension procedure with the given name, if supported and enabled.
  // The extension must have been enabled with EnableExtension.
  virtual void* GetExtensionProc(const char* extension_name,
                                 const char* proc_name) = 0;
  // Gets the extension procedure with the given name, if supported and enabled.
  // The device must be made current.
  template <typename T>
  T GetExtensionProc(const char* extension_name, const char* proc_name) {
    return reinterpret_cast<T>(GetExtensionProc(extension_name, proc_name));
  }

  // Defines the result of RecreateSurface operation.
  enum class RecreateSurfaceResult {
    // Surface created successfully and may be used.
    kSuccess,
    // The target window was not valid for the specified config.
    kInvalidTarget,
    // Memory was not available to allocate the surface. The old
    // surface may no longer be valid. Consider this fatal to the
    // context.
    kOutOfMemory,
    // The device was lost before or during recreation.
    kDeviceLost,
  };

  // Attempts to recreate the surface for the native_window.
  virtual RecreateSurfaceResult RecreateSurface(Size2D size_hint) {
    return RecreateSurfaceResult::kInvalidTarget;
  }

  // Queries the size of the backing surface, if any.
  virtual Size2D QuerySize() { return {0, 0}; }

  // Defines the swap behavior of the native window surface.
  enum class SwapBehavior {
    // Disable synchronization and swap immediately.
    // Classic no-vsync mode.
    kImmediate,
    // Synchronize to the display rate, possibly blocking for awhile.
    // Classic vsync mode.
    kSynchronize,
    // Synchronize to the display rate but allow tearing.
    // This greatly benefits variable refresh rate (VRR) displays like gsync.
    kSynchronizeAndTear,
  };

  // Sets the swap behavior for the native window surface.
  virtual void SetSwapBehavior(SwapBehavior swap_behavior) {}

  // Swaps presentation buffers, if created.
  virtual bool SwapBuffers(std::chrono::milliseconds present_time_utc_millis) {
    return false;
  }

 protected:
  friend class ThreadLock;

  ES3PlatformContext();

  // Initializes GL debugging support.
  void InitializeDebugging();
  // Initializes the list of supported extensions to enable the various
  // extension functions.
  bool InitializeExtensions();

  // Locks the context for use.
  // If the context is locked for exclusive use it will be cleared when it is
  // unlocked and otherwise it will remain current on the calling thread.
  // Returns true if the context was locked and made current.
  bool Lock(bool clear_on_unlock, std::unique_lock<std::recursive_mutex>* lock);
  // Unlocks the context, as previously locked via Lock.
  // This must be called from the same thread that performed the Lock.
  void Unlock(std::unique_lock<std::recursive_mutex> lock);

  // Used to hold the context lock.
  std::recursive_mutex usage_mutex_;
  int lock_depth_ = 0;
  bool clear_on_unlock_ = false;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_PLATFORM_CONTEXT_H_
