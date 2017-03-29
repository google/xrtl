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

#ifndef XRTL_BASE_INTRUSIVE_POOL_H_
#define XRTL_BASE_INTRUSIVE_POOL_H_

#include <climits>
#include <cstdlib>

#include "xrtl/base/intrusive_list.h"
#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"

namespace xrtl {

// Simple pool reusing IntrusiveList storage.
// Usage is similar to IntrusiveList.
//
// For items deriving from IntrusiveLinkBase (simple usage):
//   struct MyItem : public IntrusiveLinkBase<void> {};
//   IntrusivePool<MyItem> pool{8, 16};
//
// For items with list links inlined as members:
//   struct InlineItem {
//     IntrusiveListLink link;
//   };
//   IntrusivePool<InlineItem, offsetof(InlineItem, link)> pool{8, 16};
//
// Use Allocate and Release to alloc and free memory:
//   auto p = pool.Allocate();
//   pool.Release(p);
//
// Once a pointer has been released to the pool it must not be used. It's
// possible the memory will be deallocated immediately or reused for some other
// allocation, and any writes will lead to bugs.
//
// Constructors and destructors will be called as items are allocated and
// released in the pool. Constructors should always ensure they initialize all
// values to avoid bad reset behavior.
//
// Seeing issues with the pool? Change kPoolingEnabled to false and run with
// --config=asan to ensure you aren't writing-after-free.
template <typename T, size_t kOffset = 0xffffffff>
class IntrusivePoolBase {
 public:
  // Running with ASAN? Disable pooling to find errors faster.
  static constexpr bool kPoolingEnabled = true;

  // Initializes the pool to grow on demand and never free memory.
  IntrusivePoolBase() : IntrusivePoolBase(0, UINT_MAX) {}

  // Initializes the pool to contain at least min_free_size items and at most
  // max_free_size free items at a time.
  IntrusivePoolBase(size_t min_free_size, size_t max_free_size)
      : min_free_size_(min_free_size), max_free_size_(max_free_size) {
    DCHECK_LE(min_free_size, max_free_size);
    while (kPoolingEnabled && free_list_.size() < min_free_size_) {
      auto value = reinterpret_cast<T*>(std::malloc(sizeof(T)));
      auto value_link = impl::TToLink(kOffset, value);
      value_link->prev = nullptr;
      value_link->next = nullptr;
      free_list_.push_back(value);
    }
  }

  ~IntrusivePoolBase() {
    while (!free_list_.empty()) {
      auto value = free_list_.back();
      free_list_.pop_back();
      std::free(value);
    }
  }

  // Total number of free items currently in the pool.
  size_t size() const { return free_list_.size(); }

  // Allocates a new item from the pool, potentially reusing an existing value.
  // The constructor will always be called.
  T* Allocate() {
    T* value;
    if (free_list_.empty()) {
      value = reinterpret_cast<T*>(std::malloc(sizeof(T)));
    } else {
      value = free_list_.back();
      free_list_.pop_back();
    }
    auto value_link = impl::TToLink(kOffset, value);
    value_link->prev = nullptr;
    value_link->next = nullptr;
    new (value) T();
    return value;
  }

  // Releases an item to the pool, potentially stashing its value for reuse.
  // The destructor will always be called.
  void Release(T* value) {
    value->~T();
    if (kPoolingEnabled && free_list_.size() < max_free_size_) {
      free_list_.push_back(value);
    } else {
      std::free(value);
    }
  }

 private:
  IntrusiveList<T, kOffset> free_list_;

  size_t min_free_size_ = 0;
  size_t max_free_size_ = 0;

  XRTL_DISALLOW_COPY_AND_ASSIGN(IntrusivePoolBase);
};

template <typename T, size_t kOffset = 0xffffffff>
class IntrusivePool : public IntrusivePoolBase<T, kOffset> {
 public:
  using IntrusivePoolBase<T, kOffset>::IntrusivePoolBase;
};

template <typename T>
class IntrusivePool<T, 0xffffffff>
    : public IntrusivePoolBase<T, offsetof(T, link)> {
 public:
  using IntrusivePoolBase<T, offsetof(T, link)>::IntrusivePoolBase;
};

}  // namespace xrtl

#endif  // XRTL_BASE_INTRUSIVE_POOL_H_
