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

#ifndef XRTL_BASE_MAKE_UNIQUE_H_
#define XRTL_BASE_MAKE_UNIQUE_H_

#include <memory>
#include <utility>

// C++14 make_unique.
// http://herbsutter.com/2013/05/29/gotw-89-solution-smart-pointers/
// This is present in clang with -std=c++14 (in <memory>), but not
// otherwise. So define it for standard versions prior to c++14 here.

// TODO(benvanik): remove this when old GCC support is no longer required.

// Use namespace name other than 'std'
namespace xrtl {
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
#if _LIBCPP_STD_VER >= 14
  return std::make_unique<T>(std::forward<Args>(args)...);
#else
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#endif
}
}  // namespace xrtl

#endif  // XRTL_BASE_MAKE_UNIQUE_H_
