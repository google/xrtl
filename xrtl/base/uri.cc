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

#include "xrtl/base/uri.h"

namespace xrtl {
namespace uri {

bool IsSchemeless(StringView uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == 0 || first_slash == StringView::npos) {
    return true;
  }
  return uri.find_first_of("://") != first_slash - 1;
}

StringView GetScheme(StringView uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == 0 || first_slash == StringView::npos ||
      uri.find_first_of("://") != first_slash - 1) {
    return StringView();
  }
  size_t end = first_slash - 1;
  return uri.substr(0, end);
}

StringView GetHost(StringView uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == StringView::npos) {
    return StringView();
  }
  size_t start = 0;
  if (first_slash == 0 && uri.size() >= 2 && uri[1] == '/') {
    // "//...."
    start = 2;
  } else if (first_slash > 0 && uri.size() > 3 &&
             uri.find_first_of("://") == first_slash - 1) {
    // "scheme://..."
    start = first_slash + 2;
  } else {
    return StringView();
  }
  size_t end = uri.find_first_of('/', start);
  if (end == StringView::npos) {
    end = uri.size();
  }
  return uri.substr(start, end - start);
}

StringView GetOrigin(StringView uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == StringView::npos) {
    return StringView("//");
  }
  size_t scan_start = 0;
  if (first_slash == 0 && uri.size() >= 2 && uri[1] == '/') {
    // "//...."
    scan_start = 2;
  } else if (first_slash > 0 && uri.find_first_of("://") == first_slash - 1) {
    // "scheme://..."
    scan_start = first_slash + 2;
  } else {
    return StringView("//");
  }
  size_t end = uri.find_first_of('/', scan_start);
  if (end == StringView::npos) {
    end = uri.size();
  }
  return uri.substr(0, end);
}

StringView GetPath(StringView uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == StringView::npos) {
    return uri;
  }
  size_t host_start = 0;
  if (first_slash == 0 && uri.size() >= 2 && uri[1] == '/') {
    // "//...."
    host_start = 2;
  } else if (first_slash > 0 && uri.find_first_of("://") == first_slash - 1) {
    // "scheme://..."
    host_start = first_slash + 2;
  } else {
    return uri;
  }
  size_t start = uri.find_first_of('/', host_start);
  if (start == StringView::npos) {
    return StringView();
  }
  return uri.substr(start);
}

bool IsPathAbsolute(StringView path) { return !path.empty() && path[0] == '/'; }

StringView GetBasePath(StringView url) {
  size_t first_slash = url.find_first_of('/');
  if (first_slash == StringView::npos || first_slash + 2 >= url.size()) {
    return url;
  }
  first_slash = url.find_first_of('/', first_slash + 2);
  if (first_slash == StringView::npos) {
    return url;
  }

  size_t last_slash = url.find_last_of('/');
  if (last_slash == url.size() - 1) {
    last_slash = url.find_last_of('/', last_slash - 1);
  }
  if (first_slash >= last_slash && first_slash == url.size() - 1) {
    return url;
  }
  return url.substr(0, last_slash + 1);
}

std::string JoinParts(StringView left, StringView right) {
  if (left.empty()) {
    return std::string(right);
  } else if (IsPathAbsolute(right)) {
    // ... + /...
    return std::string(GetOrigin(left)) + std::string(right);
  } else if (!left.empty() && left[left.size() - 1] == '/') {
    // .../ + ...
    return std::string(left) + std::string(right);
  } else {
    // .../... + ...
    return std::string(GetBasePath(left)) + std::string(right);
  }
}

}  // namespace uri
}  // namespace xrtl
