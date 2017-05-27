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

#ifndef XRTL_BASE_FIXED_VECTOR_H_
#define XRTL_BASE_FIXED_VECTOR_H_

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"

namespace xrtl {

// Simple fixed-size list.
// This should be used in place of std::vector where the size is known and
// small. It's useful for stack allocating small arrays, inlining bounded
// dynamic arrays within other types, etc.
//
// This has roughly the same storage size as std::array but differs in that the
// size may be less than the max_size.
//
// Usage:
//   FixedVector<MyType, 5> list;
//   MyType my_type_a;
//   list.push_back(my_type_a);
//   MyType my_type_b;
//   list.push_back(my_type_b);
//   list[0]->Foo();
template <typename T, size_t kMaxSize>
class FixedVector {
 public:
  static constexpr size_t npos = -1;

  // Don't support zero capacity (neither does std::array).
  static_assert(kMaxSize > 0, "Zero capacity vectors not supported");

  FixedVector() = default;
  FixedVector(const T* data, size_t data_size) : size_(data_size) {
    DCHECK_LE(data_size, kMaxSize);
    for (size_t i = 0; i < data_size; ++i) {
      data_[i] = data[i];
    }
  }

  template <size_t N>
  FixedVector(T const (&data)[N])  // NOLINT
      : size_(N) {
    DCHECK_LE(N, kMaxSize);
    for (size_t i = 0; i < N; ++i) {
      data_[i] = data[i];
    }
  }

  template <size_t N>
  FixedVector(const std::array<T, N>& value)  // NOLINT
      : size_(value.size()) {
    DCHECK_LE(value.size(), kMaxSize);
    for (size_t i = 0; i < value.size(); ++i) {
      data_[i] = value[i];
    }
  }

  FixedVector(std::initializer_list<T> value)  // NOLINT
      : size_(value.size()) {
    DCHECK_LE(value.size(), kMaxSize);
    auto it = value.begin();
    for (size_t i = 0; i < value.size(); ++i, ++it) {
      data_[i] = *it;
    }
  }

  FixedVector(const std::vector<T>& value)  // NOLINT
      : size_(value.size()) {
    DCHECK_LE(value.size(), kMaxSize);
    for (size_t i = 0; i < value.size(); ++i) {
      data_[i] = value[i];
    }
  }

  // Returns true if the list is empty.
  bool empty() const { return size_ == 0; }
  // Total number of items in the list.
  size_t size() const { return size_; }
  // Maximum number of items in the list.
  size_t max_size() const { return kMaxSize; }
  // Contiguous backing storage for the list.
  const T* data() const { return data_; }

  // Resizes the vector to the new size.
  // The size must be <= max_size.
  void resize(size_t new_size) {
    DCHECK_LE(new_size, kMaxSize);
    for (size_t i = new_size; i < size_; ++i) {
      data_[i] = {};
    }
    size_ = new_size;
  }

  T& operator[](size_t i) {
    DCHECK_LT(i, size_);
    return data_[i];
  }
  const T& operator[](size_t i) const {
    DCHECK_LT(i, size_);
    return data_[i];
  }
  T& at(size_t i) {
    DCHECK_LT(i, size_);
    return data_[i];
  }
  const T& at(size_t i) const {
    DCHECK_LT(i, size_);
    return data_[i];
  }
  T& front() {
    DCHECK(size_);
    return data_[0];
  }
  const T& front() const {
    DCHECK(size_);
    return data_[0];
  }
  T& back() {
    DCHECK(size_);
    return data_[size_ - 1];
  }
  const T& back() const {
    DCHECK(size_);
    return data_[size_ - 1];
  }

  // Returns true if the given item is present in the list.
  bool contains(const T& item) const { return index_of(item) != npos; }

  // Returns the index of the given item or npos if it is not present in the
  // list.
  size_t index_of(const T& item) const {
    for (size_t i = 0; i < size_; ++i) {
      if (data_[i] == item) {
        return i;
      }
    }
    return npos;
  }

  // Pushes the item to the end of the list.
  // Checks if the list is out of space.
  void push_back(const T& item) {
    DCHECK_LE(size_ + 1, kMaxSize);
    data_[size_++] = item;
  }
  void push_back(T&& item) {
    DCHECK_LE(size_ + 1, kMaxSize);
    data_[size_++] = std::move(item);
  }

  void pop_back() {
    DCHECK(size_);
    data_[size_--] = {};
  }

  // Erases the item at the given index.
  // This will invalidate any active iterators.
  //
  // Performance: O(n).
  void erase_at(size_t i) {
    DCHECK_NE(-1, i);
    DCHECK_LT(i, size_);
    data_[i] = {};
    for (size_t j = i; j < size_ - 1; ++j) {
      data_[j] = std::move(data_[j + 1]);
    }
    --size_;
  }

  // Erases the given item, if it exists within the list.
  // This will invalidate any active iterators.
  //
  // Performance: O(n).
  void erase(const T& item) {
    size_t i = index_of(item);
    if (i != npos) {
      erase_at(i);
    }
  }

  // Removes all items from the list.
  // This will invalidate any active iterators.
  //
  // Performance: O(n).
  void clear() {
    for (size_t i = 0; i < size_; ++i) {
      data_[i] = {};
    }
    size_ = 0;
  }

  class iterator {
   public:
    typedef std::forward_iterator_tag iterator_category;
    explicit iterator(T* px) : px_(px) {}
    iterator operator++() {
      iterator i = *this;
      ++px_;
      return i;
    }
    iterator operator++(int n) {
      ++px_;
      return *this;
    }
    T& operator*() { return *px_; }
    T* operator->() { return px_; }
    bool operator==(const iterator& rhs) const { return px_ == rhs.px_; }
    bool operator!=(const iterator& rhs) const { return px_ != rhs.px_; }

   private:
    T* px_;
  };
  iterator begin() { return iterator(data_); }
  iterator end() { return iterator(data_ + size_); }

  class const_iterator {
   public:
    typedef std::forward_iterator_tag iterator_category;
    explicit const_iterator(const T* px) : px_(px) {}
    const_iterator operator++() {
      const_iterator i = *this;
      ++px_;
      return i;
    }
    const_iterator operator++(int n) {
      ++px_;
      return *this;
    }
    const T& operator*() { return *px_; }
    const T* operator->() { return px_; }
    bool operator==(const const_iterator& rhs) const { return px_ == rhs.px_; }
    bool operator!=(const const_iterator& rhs) const { return px_ != rhs.px_; }

   private:
    const T* px_;
  };
  const_iterator begin() const { return const_iterator(data_); }
  const_iterator end() const { return const_iterator(data_ + size_); }

 protected:
  size_t size_ = 0;
  T data_[kMaxSize];
};

}  // namespace xrtl

#endif  // XRTL_BASE_FIXED_VECTOR_H_
