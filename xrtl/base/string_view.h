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

#ifndef XRTL_BASE_STRING_VIEW_H_
#define XRTL_BASE_STRING_VIEW_H_

#include <cstring>
#include <string>

#include "xrtl/base/logging.h"

namespace xrtl {

// Classic StringPiece-alike, modelled to match a subset of C++17's StringView.
// We'll switch to that when we can.
// As much as possible methods match what will be on string_view (which in turn
// match mostly what std::string has today). In most cases dropping StringView
// in place of std::string should be possible.
//
// For documentation, see:
// http://en.cppreference.com/w/cpp/experimental/basic_string_view
class StringView {
 public:
  typedef const char* const_iterator;

  // Equivalent to std::string::npos (but to keep it a constexpr we do this).
  static constexpr size_t npos = -1;  // NOLINT

  StringView() : data_(""), size_(0) {}

  StringView(const std::string& str)  // NOLINT(runtime/explicit)
      : data_(str.data()),
        size_(str.size()) {}

  StringView(const char* data, size_t size) : data_(data), size_(size) {}

  StringView(const char* data)  // NOLINT(runtime/explicit)
      : data_(data ? data : ""),
        size_(data ? std::strlen(data) : 0) {}

  const_iterator begin() const noexcept { return data_; }
  const_iterator end() const noexcept { return data_ + size_; }

  size_t size() const noexcept { return size_; }
  bool empty() const noexcept { return size_ == 0; }

  const char& operator[](size_t i) const noexcept {
    DCHECK_LT(i, size_);
    return data_[i];
  }

  const char* data() const noexcept { return data_; }
  explicit operator std::string() const { return std::string(data_, size_); }
  bool operator!() const { return empty(); }

  StringView substr(size_t pos = 0, size_t count = npos) const;

  size_t find_first_of(const char* v, size_t pos = 0) const noexcept;
  size_t find_first_of(StringView v, size_t pos = 0) const noexcept;
  size_t find_first_of(char c, size_t pos = 0) const noexcept;

  size_t find_last_of(char c, size_t pos = npos) const noexcept;

  int compare(StringView v) const noexcept;
  inline int compare(std::string v) const noexcept {
    return compare(StringView(v));
  }
  inline int compare(const char* v) const noexcept {
    return compare(StringView(v));
  }

  // Much faster than compare() so use this if possible.
  bool equals(StringView v) const noexcept;

 private:
  const char* data_;
  size_t size_;
};

inline bool operator==(const StringView& a, const StringView& b) {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}
inline bool operator!=(const StringView& a, const StringView& b) {
  return !(a == b);
}
inline bool operator<(const StringView& a, const StringView& b) {
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}
inline bool operator>(const StringView& a, const StringView& b) {
  return b < a;
}
inline bool operator<=(const StringView& a, const StringView& b) {
  return !(b < a);
}
inline bool operator>=(const StringView& a, const StringView& b) {
  return !(a < b);
}

}  // namespace xrtl

#endif  // XRTL_BASE_STRING_VIEW_H_
