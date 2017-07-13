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

#include "xrtl/gfx/es3/es3_platform_context.h"

#include "xrtl/base/flags.h"
#include "xrtl/base/threading/thread.h"

DEFINE_bool(gl_debug_log, true, "Dump KHR_debug output to the log.");
DEFINE_bool(gl_debug_log_synchronous, true,
            "KHR_debug will synchronize to be thread safe.");

namespace xrtl {
namespace gfx {
namespace es3 {

namespace {

// Thread-locked context storage.
// The context is retained by the local storage until the thread exits or
// ReleaseThreadContext is called.
Thread::LocalStorageSlot<ES3PlatformContext> thread_context_slot_{
    [](ES3PlatformContext* thread_context) {
      // Clear the context so it's not bound.
      thread_context->ClearCurrent();

      // Drop reference - this may delete the thread context.
      thread_context->ReleaseReference();
    }};

// Currently locked context on the thread. May be null if none is locked.
Thread::LocalStorageSlot<ES3PlatformContext> locked_context_slot_{
    [](ES3PlatformContext* thread_context) {
      // Clear the context so it's not bound.
      thread_context->ClearCurrent();

      // Drop reference - this may delete the thread context.
      thread_context->ReleaseReference();
    }};

extern "C" void OnDebugMessage(GLenum source, GLenum type, GLuint id,
                               GLenum severity, GLsizei length,
                               const GLchar* message, GLvoid* user_param) {
  const char* source_name = "Unknown";
  switch (source) {
    case GL_DEBUG_SOURCE_API:
      source_name = "OpenGL";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      source_name = "Windows";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      source_name = "Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      source_name = "Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      source_name = "Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      source_name = "Other";
      break;
  }

  const char* type_name = "unknown";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      type_name = "error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      type_name = "deprecated behavior";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      type_name = "undefined behavior";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      type_name = "portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      type_name = "performance";
      break;
    case GL_DEBUG_TYPE_OTHER:
      type_name = "message";
      break;
    case GL_DEBUG_TYPE_MARKER:
      type_name = "marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      type_name = "push group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      type_name = "pop group";
      break;
  }

  int glog_severity = ::xrtl::INFO;
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      glog_severity = ::xrtl::ERROR;
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      glog_severity = ::xrtl::WARNING;
      break;
    case GL_DEBUG_SEVERITY_LOW:
      glog_severity = ::xrtl::INFO;
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      glog_severity = ::xrtl::INFO;
      break;
  }

  ::xrtl::internal::LogMessage(__FILE__, __LINE__, glog_severity)
      << "GL::" << source_name << ": " << type_name << " #" << id << ", "
      << message;
}

}  // namespace

ES3PlatformContext::ES3PlatformContext() = default;

ES3PlatformContext::~ES3PlatformContext() = default;

ref_ptr<ES3PlatformContext> ES3PlatformContext::AcquireThreadContext(
    ref_ptr<ES3PlatformContext> existing_context) {
  // Check the current TLS to see if we have a thread-locked context.
  ref_ptr<ES3PlatformContext> thread_context(thread_context_slot_.value());
  if (thread_context) {
    // Reuse existing thread context.
    return thread_context;
  }

  // Attempt to create a new context for the thread.
  thread_context = Create(std::move(existing_context));
  if (!thread_context) {
    LOG(ERROR) << "Unable to create a new thread-locked context";
    return nullptr;
  }

  // Stash context for later use.
  thread_context->AddReference();
  thread_context_slot_.set_value(thread_context.get());

  // We can't trust TLS to clear us at the right time, so do it ourselves if
  // needed. It may be a no-op but it's safer than doing nothing.
  Thread::current_thread()->RegisterExitCallback(
      []() { ReleaseThreadContext(); });

  return thread_context;
}

void ES3PlatformContext::ReleaseThreadContext() {
  // Ensure the context is still valid. It may have been destroyed.
  ref_ptr<ES3PlatformContext> thread_context{thread_context_slot_.value()};
  if (!thread_context) {
    // No thread-locked context - ignore.
    return;
  }

  // Clear the TLS slot.
  thread_context->ReleaseReference();
  thread_context_slot_.set_value(nullptr);

  // Clear the context so it's not bound.
  thread_context->ClearCurrent();

  // NOTE: if the TLS was holding on to the last reference it'll now be
  //       destroyed.
}

ES3PlatformContext::ThreadLock ES3PlatformContext::LockTransientContext(
    ref_ptr<ES3PlatformContext> existing_context) {
  // See if we already have a context locked. If so, we can just reuse that.
  ref_ptr<ES3PlatformContext> locked_context{locked_context_slot_.value()};
  if (locked_context) {
    return ThreadLock(locked_context);
  }

  // No currently locked context, use the thread-locked one.
  return ThreadLock(AcquireThreadContext(std::move(existing_context)));
}

bool ES3PlatformContext::Lock(bool clear_on_unlock,
                              std::unique_lock<std::recursive_mutex>* lock) {
  std::unique_lock<std::recursive_mutex> lock_guard(usage_mutex_);
  ++lock_depth_;

  // If you're getting an error here it means that some other context is locked
  // while this lock was attempted. Don't do that.
  DCHECK(lock_depth_ == 1 || IsCurrent());
  ref_ptr<ES3PlatformContext> locked_context{locked_context_slot_.value()};
  DCHECK(!locked_context || locked_context.get() == this);

  if (lock_depth_ == 1) {
    // The first lock decides whether this is really exclusive or thread-local.
    clear_on_unlock_ = clear_on_unlock;

    // Stash in TLS for reuse.
    AddReference();
    locked_context_slot_.set_value(this);

    if (!MakeCurrent()) {
      --lock_depth_;
      return false;
    }
  }
  *lock = std::move(lock_guard);
  return true;
}

void ES3PlatformContext::Unlock(std::unique_lock<std::recursive_mutex> lock) {
  DCHECK_GE(lock_depth_, 1);
  --lock_depth_;
  if (lock_depth_ == 0) {
    // NOTE: the flush is required to ensure changes on this context make it
    // to other contexts.
    Flush();
    if (clear_on_unlock_) {
      ClearCurrent();
    }

    // Clear the TLS slot.
    ref_ptr<ES3PlatformContext> locked_context{locked_context_slot_.value()};
    DCHECK(locked_context && locked_context.get() == this);
    ReleaseReference();
    locked_context_slot_.set_value(nullptr);
  }
  lock.unlock();
}

void ES3PlatformContext::InitializeDebugging() {
  if (glDebugMessageCallback == nullptr) {
    // Not supported; ignore.
    return;
  }

  glEnable(GL_DEBUG_OUTPUT);

  // Synchronous logging makes log outputs easier to read.
  if (FLAGS_gl_debug_log_synchronous) {
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  } else {
    glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }

  // Enable everything by default.
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL,
                        GL_TRUE);

  // Disable messages that don't mean much for us.
  GLuint disable_message_ids[] = {
      131185,  // Buffer detailed info : Buffer object 1 (bound to
               // GL_ARRAY_BUFFER_ARB, usage hint is GL_STREAM_DRAW) will use
               // VIDEO memory as the source for buffer object operations.
  };
  glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE,
                        static_cast<GLsizei>(count_of(disable_message_ids)),
                        disable_message_ids, GL_FALSE);

  // Callback will be made from driver threads.
  glDebugMessageCallback(reinterpret_cast<GLDEBUGPROC>(&OnDebugMessage), this);
}

bool ES3PlatformContext::InitializeExtensions() {
  // Initialize debugging API, if we want it.
  // We should do this ASAP to start getting enhanced logging.
  if (FLAGS_gl_debug_log) {
    InitializeDebugging();
  }

  // TODO(benvanik): extension support.
  return true;
}

bool ES3PlatformContext::IsExtensionSupported(const char* extension_name) {
  // TODO(benvanik): extension support.
  return false;
}

bool ES3PlatformContext::IsExtensionEnabled(const char* extension_name) {
  // TODO(benvanik): extension support.
  return false;
}

bool ES3PlatformContext::EnableExtension(const char* extension_name) {
  if (!IsExtensionSupported(extension_name)) {
    return false;
  } else if (IsExtensionEnabled(extension_name)) {
    return true;
  }
  if (TryEnableExtension(extension_name)) {
    // TODO(benvanik): set extension enabled.
    return true;
  }
  return false;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
