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

#ifndef XRTL_BASE_ENV_H_
#define XRTL_BASE_ENV_H_

#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "xrtl/base/macros.h"

namespace xrtl {

// System environment utilities.
// These abstract common environment-dependent APIs across platforms. Always
// prefer using these over direct STL/posix/etc APIs.
class Env {
 public:
  // Returns the value of the given environment variable, if it exists.
  // This is equivalent to std::getenv.
  static absl::optional<std::string> GetValue(absl::string_view key);

  // Returns the value of the given environment variable or the provided default
  // if it doesn't exist.
  static std::string GetValueOrDefault(absl::string_view key,
                                       absl::string_view default_value);

  // Returns a path that can be used for temporary files.
  // It is guaranteed to exist and be writable, though the amount of storage
  // space available is undefined.
  static std::string temp_path();
};

}  // namespace xrtl

#endif  // XRTL_BASE_ENV_H_
