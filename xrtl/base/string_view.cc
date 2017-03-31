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

#include "xrtl/base/string_view.h"

#include <algorithm>

namespace xrtl {

namespace {

// These two functions come from //strings, as they are not available in most
// C runtimes. The functions available in STL (std::strstr, etc) are not safe
// for our StringView operations where the strings may not be null-terminated.

inline char* strnchr(const char* buf, char c, size_t sz) {
  const char* end = buf + sz;
  while (buf != end && *buf) {
    if (*buf == c) return const_cast<char*>(buf);
    ++buf;
  }
  return nullptr;
}

char* strnstr(const char* haystack, const char* needle, size_t haystack_len) {
  if (*needle == '\0') {
    return const_cast<char*>(haystack);
  }
  size_t needle_len = strlen(needle);
  char* where;
  while ((where = strnchr(haystack, *needle, haystack_len)) != nullptr) {
    if (where - haystack + needle_len > haystack_len) {
      return nullptr;
    }
    if (strncmp(where, needle, needle_len) == 0) {
      return where;
    }
    haystack_len -= where + 1 - haystack;
    haystack = where + 1;
  }
  return nullptr;
}

}  // namespace

StringView StringView::substr(size_t pos, size_t count) const {
  pos = std::min(pos, size_);
  return StringView(data_ + pos, count != npos ? std::min(count, size() - pos)
                                               : size() - pos);
}

size_t StringView::find_first_of(const char* v, size_t pos) const noexcept {
  // TODO(benvanik): can we use std::find* here?
  pos = std::min(pos, size_);
  auto result = strnstr(data_ + pos, v, size_ - pos);
  return result ? static_cast<size_t>(result - data_) : npos;
}

size_t StringView::find_first_of(StringView v, size_t pos) const noexcept {
  // TODO(benvanik): optimize using length information.
  return find_first_of(v.data(), pos);
}

size_t StringView::find_first_of(char c, size_t pos) const noexcept {
  if (pos >= size_) {
    return npos;
  }
  pos = std::min(pos, size_);
  for (size_t i = pos; i < size_; ++i) {
    if (data_[i] == c) {
      return i;
    }
  }
  return npos;
}

size_t StringView::find_last_of(char c, size_t pos) const noexcept {
  pos = pos == npos ? size_ - 1 : pos;
  pos = std::min(pos, size_);
  for (int i = pos; i >= 0; --i) {
    if (data_[i] == c) {
      return i;
    }
  }
  return npos;
}

int StringView::compare(StringView v) const noexcept {
  size_t count = std::min(size_, v.size());
  if (count == 0) {
    return size_ == v.size() ? 0 : 1;
  }
  return std::strncmp(v.data(), data_, count);
}

bool StringView::equals(StringView v) const noexcept {
  if (size_ != v.size_) {
    return false;
  } else if (data_ == v.data()) {
    return true;
  } else if (size_ == 0) {
    return true;
  }
  return std::memcmp(data_, v.data(), size_) == 0;
}

}  // namespace xrtl
