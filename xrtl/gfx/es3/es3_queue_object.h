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

#ifndef XRTL_GFX_ES3_ES3_QUEUE_OBJECT_H_
#define XRTL_GFX_ES3_ES3_QUEUE_OBJECT_H_

#include <utility>

#include "xrtl/base/ref_ptr.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

// Interface for objects that are managed by an ES3Queue.
// Implementing this allows an object to have its lifetime lazily managed on the
// queue. This is used for GL objects where otherwise a GL context would be
// required on any thread attempting to alloc/dealloc an object.
//
// Objects implementing this interface must take care to make themselves either
// immutable after initial creation or thread-safe. They must also be consistent
// in their usage of the queue and not try to work around it (by, say, issuing
// GL calls anywhere but in the callbacks defined by the interface).
//
// For example, if you had an object owning a GL resource as follows:
//   class Foo {
//     Foo() { my_id_ = glCreateFoo(); }
//     ~Foo() { glDeleteFoo(my_id_); }
//     int some_query() { return glGet(GL_FOO, my_id_); }
//     int SyncCall() { return glFoo(my_id_); }
//   };
// The queue variant would look like:
//   class Foo : public ManagedObject, public ES3QueueObject {
//     explicit Foo(ES3ObjectLifetimeQueue* queue) : queue_(queue) {
//       queue_->EnqueueObjectAllocation(this);
//     }
//     void Release() override { queue_->EnqueueObjectDeallocation(this); }
//     bool AllocateOnQueue() {
//       my_id_ = glCreateFoo();
//       cached_value_ = glGet(GL_FOO, my_id_);
//     }
//     void DeallocateOnQueue() { glDeleteFoo(my_id_); }
//     int some_query() { return cached_value_; }
//     int SyncCall() {
//       int value = 0;
//       queue_->SyncObjectCallback(this, [&value]() {
//         value = glFoo(my_id_);
//       });
//       return value;
//     }
//   };
//
// Code creating queue objects must call PrepareAllocation after they have
// created the instance:
//   auto my_object = make_ref<MyObject>(...);
//   my_object->PrepareAllocation();
//   return my_object;
class ES3QueueObject {
 public:
  virtual ~ES3QueueObject() = default;

  // Prepares the object for use by queueing allocation work on the main thread.
  // This must be called outside of the constructor for the object.
  // Implementation contents are usually something like:
  //   void MyObject::PrepareAllocation() {
  //     queue_->EnqueueObjectAllocation(this);
  //   }
  virtual void PrepareAllocation() = 0;

 protected:
  friend class ES3ObjectLifetimeQueue;

  ES3QueueObject() = default;

  // Allocates the object from the queue thread.
  // This is called once per object prior to validation/first use.
  // A GL context will be locked and available for use during the call.
  // Returns true if the object was allocated successfully and is available for
  // use.
  virtual bool AllocateOnQueue() XRTL_REQUIRES_GL_CONTEXT = 0;

  // Deallocates the object from the queue thread.
  // This is called once per object and always after allocation. This is
  // effectively the destructor and no future use of the object will be made
  // by the queue or any command buffers.
  // A GL context will be locked and available for use during the call.
  virtual void DeallocateOnQueue() XRTL_REQUIRES_GL_CONTEXT = 0;
};

// Interface for work queues that are used to manage object lifetime.
// ES3QueueObjects use these queues to run their allocation routines and
// callbacks that require a locked platform context to operate.
class ES3ObjectLifetimeQueue : public RefObject<ES3ObjectLifetimeQueue> {
 public:
  using ObjectReleaseCallback = void (*)(ES3QueueObject*);

  virtual ~ES3ObjectLifetimeQueue() = default;

  // Enqueues an object for allocation on the queue.
  // The ES3QueueObject::AllocateOnQueue method will be called before any
  // following queued commands execute.
  //
  // If provided a custom platform context can be specified for locking prior
  // to executing the allocation; otherwise the queue context is used.
  template <typename T>
  void EnqueueObjectAllocation(
      T* obj, ref_ptr<ES3PlatformContext> exclusive_context = nullptr) {
    obj->AddReference();
    EnqueueObjectCallback(
        static_cast<ES3QueueObject*>(obj),
        [](ES3QueueObject* obj) { static_cast<T*>(obj)->ReleaseReference(); },
        [obj]() { static_cast<ES3QueueObject*>(obj)->AllocateOnQueue(); },
        std::move(exclusive_context));
  }

  // Enqueues an object for deallocation on the queue.
  // The ES3QueueObject::DeallocateOnQueue method will be called before any
  // following queued commands execute. The object will be deleted prior to the
  // return of the function.
  //
  // If provided a custom platform context can be specified for locking prior
  // to executing the deallocation; otherwise the queue context is used.
  template <typename T>
  void EnqueueObjectDeallocation(
      T* obj, ref_ptr<ES3PlatformContext> exclusive_context = nullptr) {
    EnqueueObjectCallback(
        static_cast<ES3QueueObject*>(obj),
        [](ES3QueueObject* obj) { delete static_cast<T*>(obj); },
        [obj]() { static_cast<ES3QueueObject*>(obj)->DeallocateOnQueue(); },
        std::move(exclusive_context));
  }

  // Enqueues an asynchronous object-specific callback on the queue.
  // The callback will be called before any following queued commands execute.
  //
  // If provided a custom platform context can be specified for locking prior
  // to executing the callback; otherwise the queue context is used.
  template <typename T>
  void EnqueueObjectCallback(
      T* obj, std::function<void()> callback,
      ref_ptr<ES3PlatformContext> exclusive_context = nullptr) {
    obj->AddReference();
    EnqueueObjectCallback(
        static_cast<ES3QueueObject*>(obj),
        [](ES3QueueObject* obj) { static_cast<T*>(obj)->ReleaseReference(); },
        std::move(callback), std::move(exclusive_context));
  }

  // Enqueues a synchronous object-specific callback on the queue.
  // The callback will be called before any following queued commands execute.
  // Returns the result of the callback or false if the callback could not be
  // executed.
  //
  // If provided a custom platform context can be specified for locking prior
  // to executing the callback; otherwise the queue context is used.
  template <typename T>
  bool EnqueueObjectCallbackAndWait(
      T* obj, std::function<bool()> callback,
      ref_ptr<ES3PlatformContext> exclusive_context = nullptr) {
    return SyncObjectCallback(static_cast<ES3QueueObject*>(obj),
                              std::move(callback),
                              std::move(exclusive_context));
  }

 protected:
  ES3ObjectLifetimeQueue() = default;

  virtual void EnqueueObjectCallback(
      ES3QueueObject* obj, ObjectReleaseCallback release_callback,
      std::function<void()> callback,
      ref_ptr<ES3PlatformContext> exclusive_context) = 0;
  virtual bool SyncObjectCallback(
      ES3QueueObject* obj, std::function<bool()> callback,
      ref_ptr<ES3PlatformContext> exclusive_context) = 0;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_QUEUE_OBJECT_H_
