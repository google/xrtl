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

#include "xrtl/base/threading/thread.h"

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
  thread_context = Create(existing_context);
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

bool ES3PlatformContext::Lock(bool clear_on_unlock,
                              std::unique_lock<std::recursive_mutex>* lock) {
  std::unique_lock<std::recursive_mutex> lock_guard(usage_mutex_);
  ++lock_depth_;

  // If you're getting an error here it means that some other context is locked
  // while this lock was attempted. Don't do that.
  DCHECK(lock_depth_ == 1 || IsCurrent());

  if (lock_depth_ == 1) {
    // The first lock decides whether this is really exclusive or thread-local.
    clear_on_unlock_ = clear_on_unlock;
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
    if (clear_on_unlock_) {
      ClearCurrent();
    }
  }
  lock.unlock();
}

bool ES3PlatformContext::InitializeExtensions() {
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
