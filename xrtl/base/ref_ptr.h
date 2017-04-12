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

#ifndef XRTL_BASE_REF_PTR_H_
#define XRTL_BASE_REF_PTR_H_

#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"

namespace xrtl {

// Use this to get really verbose refptr logging:
// #define XRTL_VERBOSE_REF_PTR

// Reference counted pointer container.
// This is modeled on boost::instrusive_ptr in that it requires no
// extra storage over the pointer type and should compile to almost
// no additional code. It also allows us to round-trip object pointers
// through regular pointers, which is critical when having to round-trip
// them through JNI/etc where we can't use things like unique_ptr/shared_ptr.
//
//   ref_ptr<Foo> p1(new Foo());    // ref count 1
//   ref_ptr<Foo> p2(p1);           // ref count 2
//   p1.reset();                    // ref count 1
//   p2.reset();                    // ref count 0, deleted
//
// When round-tripping the pointer through external APIs, use release():
//   ref_ptr<Foo> p1(new Foo());    // ref count 1
//   Foo* raw_p = p1.release();     // ref count 1
//   // pass to API
//   ref_ptr<Foo> p2(raw_p, false); // ref count 1 (false = don't add ref)
//   p2.reset();                    // ref count 0, deleted
//
// See the boost intrusive_ptr docs for details of behavior:
// http://www.boost.org/doc/libs/1_55_0/libs/smart_ptr/intrusive_ptr.html
//
// ref_ptr manages the target objects in a thread-safe way, though you'll want
// to take care with objects that may have pinned threads for deallocation. If
// you release the last reference to an object on a thread other than what it
// was expecting you're gonna have a bad time.
//
// Compatible only with types that subclass RefObject or implement the following
// methods:
//   ref_ptr_add_ref
//   ref_ptr_release_ref
template <class T>
class ref_ptr {
 private:
  typedef ref_ptr this_type;
  typedef T* this_type::*unspecified_bool_type;

 public:
  // Initializes with nullptr.
  ref_ptr() : px_(nullptr) {}

  // Initializes with nullptr so that there is no way to create an
  // uninitialized ref_ptr.
  ref_ptr(std::nullptr_t) : px_(nullptr) {}  // NOLINT

  // Initializes the pointer to the given value, optionally adding to the
  // reference count.
  explicit ref_ptr(T* p, bool add_ref = true) : px_(p) {
    if (px_ && add_ref) {
      ref_ptr_add_ref(px_);
    }
  }

  // Initializes the pointer to the same object as the given reference,
  // incrementing its reference count.
  // This variant supports subclasses.
  template <class U>
  ref_ptr(ref_ptr<U> const& rhs)  // NOLINT
      : px_(rhs.get()) {
    ref_ptr_add_ref(px_);
  }

  // Initializes the pointer to the same object as the given reference,
  // incrementing its reference count.
  ref_ptr(ref_ptr const& rhs) : px_(rhs.px_) { ref_ptr_add_ref(px_); }

  // Decrements the reference count of the owned pointer.
  ~ref_ptr() { ref_ptr_release_ref(px_); }

  // Like the ref_ptr(ref_ptr<U>&) constructor.
  template <class U>
  ref_ptr& operator=(ref_ptr<U> const& rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }
  // Like the ref_ptr(ref_ptr&) constructor.
  ref_ptr& operator=(ref_ptr const& rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }
  // Like reset().
  ref_ptr& operator=(T* rhs) {
    this_type(rhs).swap(*this);
    return *this;
  }

  XRTL_ALLOW_RVALUE_REFERENCES_PUSH
  // std::move support. Requires -Wgoogle3-rvalue-reference.
  // && is against style guide but required for proper move support for ptr
  // types (and since the style guide allows std::move for unique_ptr/etc it
  // should be allowed here too).
  ref_ptr(ref_ptr&& rhs) : px_(rhs.px_) { rhs.px_ = 0; }  // NOLINT
  ref_ptr& operator=(ref_ptr&& rhs) {                     // NOLINT
    this_type(std::move(rhs)).swap(*this);
    return *this;
  }
  XRTL_ALLOW_RVALUE_REFERENCES_POP

  // Resets the object to nullptr and decrements the reference count, possibly
  // deleting it.
  void reset() { this_type().swap(*this); }
  // Resets the object to the given pointer, decrementing the reference count of
  // the existing object and incrementing the reference count of the given one.
  void reset(T* rhs) { this_type(rhs).swap(*this); }

  // Releases a pointer.
  // Returns the current pointer held by this object without having
  // its reference count decremented and resets the ref_ptr to empty.
  // Returns nullptr if the ref_ptr holds no value.
  // To re-wrap in a ref_ptr use either ref_ptr<T>(value, false) or assign().
  T* release() {
    T* p = px_;
    px_ = nullptr;
    return p;
  }

  // Assigns a pointer.
  // The pointer will be accepted by the ref_ptr and its reference count will
  // not be incremented.
  void assign(T* value) {
    reset();
    px_ = value;
  }

  // Gets the pointer referenced by this instance.
  // operator* and operator-> will assert() if there is no current object.
  T* get() const { return px_; }
  T& operator*() const {
    DCHECK(px_);
    return *px_;
  }
  T* operator->() const {
    DCHECK(px_);
    return px_;
  }

  // Supports boolean expression evaluation.
  operator unspecified_bool_type() const { return px_ ? &this_type::px_ : 0; }
  // Supports unary expression evaluation.
  bool operator!() const { return !px_; }

  // Swap support.
  void swap(ref_ptr& rhs) { std::swap(px_, rhs.px_); }

  // Casts from one ref_ptr type to another.
  template <typename U>
  ref_ptr<U> As() const {
    return ref_ptr<U>(static_cast<U*>(px_), true);
  }

 private:
  T* px_;
};

// Base class for reference counted objects.
// Reference counted objects should be used with the ref_ptr pointer type.
// As reference counting can be tricky always prefer to use unique_ptr and
// avoid this type. Only use this when unique_ptr is not possible, such as
// when round-tripping objects through marshaling boundaries (v8/Java) or
// any objects that may have their lifetime tied to a garbage collected
// object.
//
// Subclasses should protect their dtor so that reference counting must
// be used.
//
// This is designed to avoid the need for extra vtable space or for adding
// methods to the vtable of subclasses. This differs from the boost Pointable
// version of this object.
// Inspiration for this comes from Peter Weinert's Dr. Dobb's article:
// http://www.drdobbs.com/cpp/a-base-class-for-intrusively-reference-c/229218807
//
// RefObjects are thread safe and may be used with ref_ptrs from multiple
// threads.
//
// Subclasses may implement a custom Delete operator to handle their
// deallocation. It should be thread safe as it may be called from any thread.
//
// Usage:
//   class MyRefObject : public RefObject<MyRefObject> {
//    public:
//     MyRefObject() = default;
//     // Optional; can be used to return to pool/etc - must be public:
//     static void Delete(MyRefObject* ptr) {
//       ::operator delete(ptr);
//     }
//   };
template <class T>
class RefObject {
  static_assert(!std::is_array<T>::value, "T must not be an array");

  // value is true if a static Delete(T*) function is present.
  template <typename V>
  struct has_custom_deleter {
    template <typename U, U>
    struct Check;
    template <typename U>
    static std::true_type Test(Check<void (*)(V*), &U::Delete>*);
    template <typename U>
    static std::false_type Test(...);
    static const bool value = decltype(Test<V>(nullptr))::value;
  };

  template <typename V, bool has_custom_deleter>
  struct delete_thunk {
    static void Delete(V* p) {
      auto ref_obj = static_cast<RefObject<V>*>(p);
      int previous_count = ref_obj->counter_.fetch_sub(1);
#ifdef XRTL_VERBOSE_REF_PTR
      LOG(INFO) << "ro-- " << typeid(V).name() << " " << p << " now "
                << previous_count - 1
                << (previous_count == 1 ? " DEAD (CUSTOM)" : "");
#endif  // XRTL_VERBOSE_REF_PTR
      if (previous_count == 1) {
        // We delete type T pointer here to avoid the need for a virtual dtor.
        V::Delete(p);
      }
    }
  };

  template <typename V>
  struct delete_thunk<V, false> {
    static void Delete(V* p) {
      auto ref_obj = static_cast<RefObject<V>*>(p);
      int previous_count = ref_obj->counter_.fetch_sub(1);
#ifdef XRTL_VERBOSE_REF_PTR
      LOG(INFO) << "ro-- " << typeid(V).name() << " " << p << " now "
                << previous_count - 1 << (previous_count == 1 ? " DEAD" : "");
#endif  // XRTL_VERBOSE_REF_PTR
      if (previous_count == 1) {
        // We delete type T pointer here to avoid the need for a virtual dtor.
        delete p;
      }
    }
  };

 public:
  // Adds a reference; used by ref_ptr.
  friend void ref_ptr_add_ref(T* p) {
    if (!p) {
      return;
    }
    auto ref_obj = static_cast<RefObject*>(p);
    ++ref_obj->counter_;

#ifdef XRTL_VERBOSE_REF_PTR
    LOG(INFO) << "ro++ " << typeid(T).name() << " " << p << " now "
              << ref_obj->counter_;
#endif  // XRTL_VERBOSE_REF_PTR
  }

  // Releases a reference, potentially deleting the object; used by ref_ptr.
  friend void ref_ptr_release_ref(T* p) {
    if (p) {
      delete_thunk<T, has_custom_deleter<T>::value>::Delete(p);
    }
  }

  // Adds a reference.
  // ref_ptr should be used instead of this in most cases. This is required
  // for when interoperating with marshaling APIs.
  void AddReference() { ref_ptr_add_ref(static_cast<T*>(this)); }

  // Releases a reference, potentially deleting the object.
  // ref_ptr should be used instead of this in most cases. This is required
  // for when interoperating with marshaling APIs.
  void ReleaseReference() { ref_ptr_release_ref(static_cast<T*>(this)); }

 protected:
  RefObject() {}
  RefObject(const RefObject&) = default;
  RefObject& operator=(const RefObject&) { return *this; }

  // Returns true if the object is deleted and undergoing destruction.
  bool is_deleted() const { return !counter_; }

  std::atomic<int> counter_{0};
};

// Various comparison operator overloads.

template <class T, class U>
inline bool operator==(ref_ptr<T> const& a, ref_ptr<U> const& b) {
  return a.get() == b.get();
}

template <class T, class U>
inline bool operator!=(ref_ptr<T> const& a, ref_ptr<U> const& b) {
  return a.get() != b.get();
}

template <class T, class U>
inline bool operator==(ref_ptr<T> const& a, U* b) {
  return a.get() == b;
}

template <class T, class U>
inline bool operator!=(ref_ptr<T> const& a, U* b) {
  return a.get() != b;
}

template <class T, class U>
inline bool operator==(T* a, ref_ptr<U> const& b) {
  return a == b.get();
}

template <class T, class U>
inline bool operator!=(T* a, ref_ptr<U> const& b) {
  return a != b.get();
}

template <class T>
inline bool operator<(ref_ptr<T> const& a, ref_ptr<T> const& b) {
  return a.get() < b.get();
}

// Swaps the pointers of two ref_ptrs.
template <class T>
void swap(ref_ptr<T>& lhs, ref_ptr<T>& rhs) {
  lhs.swap(rhs);
}

// Allocates a new ref_ptr type.
// Like make_unique, but for ref_ptr.
//
// Usage:
//   ref_ptr<MyType> p = make_ref<MyType>(1, 2, 3);
template <typename T, typename... Args>
ref_ptr<T> make_ref(Args&&... args) {
  return ref_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace xrtl

#endif  // XRTL_BASE_REF_PTR_H_
