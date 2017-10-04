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

bool IsSchemeless(absl::string_view uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == 0 || first_slash == absl::string_view::npos) {
    return true;
  }
  return uri.find_first_of("://") != first_slash - 1;
}

absl::string_view GetScheme(absl::string_view uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == 0 || first_slash == absl::string_view::npos ||
      uri.find_first_of("://") != first_slash - 1) {
    return absl::string_view();
  }
  size_t end = first_slash - 1;
  return uri.substr(0, end);
}

absl::string_view GetHost(absl::string_view uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == absl::string_view::npos) {
    return absl::string_view();
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
    return absl::string_view();
  }
  size_t end = uri.find_first_of('/', start);
  if (end == absl::string_view::npos) {
    end = uri.size();
  }
  return uri.substr(start, end - start);
}

absl::string_view GetOrigin(absl::string_view uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == absl::string_view::npos) {
    return absl::string_view("//");
  }
  size_t scan_start = 0;
  if (first_slash == 0 && uri.size() >= 2 && uri[1] == '/') {
    // "//...."
    scan_start = 2;
  } else if (first_slash > 0 && uri.find_first_of("://") == first_slash - 1) {
    // "scheme://..."
    scan_start = first_slash + 2;
  } else {
    return absl::string_view("//");
  }
  size_t end = uri.find_first_of('/', scan_start);
  if (end == absl::string_view::npos) {
    end = uri.size();
  }
  return uri.substr(0, end);
}

absl::string_view GetPath(absl::string_view uri) {
  size_t first_slash = uri.find_first_of('/');
  if (first_slash == absl::string_view::npos) {
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
  if (start == absl::string_view::npos) {
    return absl::string_view();
  }
  return uri.substr(start);
}

bool IsPathAbsolute(absl::string_view path) {
  return !path.empty() && path[0] == '/';
}

absl::string_view GetBasePath(absl::string_view url) {
  size_t first_slash = url.find_first_of('/');
  if (first_slash == absl::string_view::npos || first_slash + 2 >= url.size()) {
    return url;
  }
  first_slash = url.find_first_of('/', first_slash + 2);
  if (first_slash == absl::string_view::npos) {
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

std::string JoinParts(absl::string_view left, absl::string_view right) {
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
