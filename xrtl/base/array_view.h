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

#ifndef XRTL_BASE_ARRAY_VIEW_H_
#define XRTL_BASE_ARRAY_VIEW_H_

#include <array>
#include <vector>

#include "xrtl/base/logging.h"
#include "xrtl/base/macros.h"

namespace xrtl {

// An immutable view into a slice of an array type.
// This can be used to easily allow methods to take a variety of array forms
// without needing to provide a bunch of overloads. The array view does not
// copy or take ownership of the data it references and callers must ensure the
// data remains valid (on the stack or heap) until use of the array view is
// ended. In general, you shouldn't use this as a class member value.
//
// Usage:
//  ArrayView<int> my_view({1, 2, 3});
//  EXPECT_EQ(3, my_view.size());
//  EXPECT_EQ(2, my_view[1]);
template <typename T>
class ArrayView {
 public:
  using const_iterator = const T*;
  typedef const T* ArrayView<T>::*unspecified_bool_type;

  ArrayView() = default;
  ArrayView(const T* data, size_t data_size)
      : size_(data_size), data_(data_size ? data : nullptr) {}
  template <size_t N>
  ArrayView(T const (&data)[N])  // NOLINT
      : size_(N), data_(N ? data : nullptr) {}
  template <size_t N>
  ArrayView(const std::array<T, N>& value)  // NOLINT
      : size_(N), data_(N ? value.data() : nullptr) {}
  ArrayView(std::initializer_list<T> value)  // NOLINT
      : size_(value.size()), data_(value.size() ? value.begin() : nullptr) {}
  ArrayView(const std::vector<T>& value)  // NOLINT
      : size_(value.size()), data_(value.size() ? value.data() : nullptr) {}

  const_iterator begin() const noexcept { return data_; }
  const_iterator end() const noexcept {
    return data_ ? data_ + size_ : nullptr;
  }

  // True if the array view is empty.
  bool empty() const noexcept { return size_ == 0; }
  // The number of items in the array view.
  size_t size() const noexcept { return size_; }
  // Returns a pointer to the contiguous item data.
  // This may not exist beyond the current call stack and should not be
  // retained.
  const T* data() const noexcept { return data_; }

  // Accesses an item in the array view.
  // Must be called with a valid index in [0, size()).
  const T& operator[](size_t i) const noexcept {
    DCHECK_LT(i, size_);
    return data_[i];
  }

  // Supports boolean expression evaluation.
  operator unspecified_bool_type() const {
    return size_ > 0 ? &ArrayView<T>::data_ : 0;
  }
  // Supports unary expression evaluation.
  bool operator!() const { return size_ == 0; }

  // Conversion into std::vector for storage.
  // The data elements will be copied, not moved.
  operator std::vector<T>() const {
    std::vector<T> result;
    result.resize(size_);
    for (size_t i = 0; i < size_; ++i) {
      result[i] = data_[i];
    }
    return result;
  }

 private:
  size_t size_ = 0;
  const T* data_ = nullptr;
};

}  // namespace xrtl

#endif  // XRTL_BASE_ARRAY_VIEW_H_
