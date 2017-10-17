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

#include <cstdlib>

#include "xrtl/base/env.h"
#include "xrtl/base/logging.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {

absl::optional<std::string> Env::GetValue(absl::string_view key) {
  char key_buffer[255 + 1] = {0};
  if (key.size() >= ABSL_ARRAYSIZE(key_buffer)) {
    DCHECK(false) << "Attempting to access a really long env var: " << key;
    return absl::nullopt;
  }
  std::memcpy(key_buffer, key.data(), key.size());
  key_buffer[key.size()] = 0;

  // Note that this query returns count+1 (for NUL).
  DWORD chars_required = ::GetEnvironmentVariableA(key_buffer, nullptr, 0);
  if (chars_required == 0) {
    if (::GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
      // Environment variable not found (but query was OK).
      return absl::nullopt;
    }
    DCHECK(false) << "Unable to query environment variable " << key << " size";
    return absl::nullopt;
  }

  std::string value;
  value.resize(chars_required - 1);

  DWORD chars_read =
      ::GetEnvironmentVariableA(key_buffer, &value[0], value.size() + 1);
  if (chars_read == 0) {
    DCHECK(false) << "Unable to read environment variable " << key;
    return absl::nullopt;
  }

  return value;
}

std::string Env::temp_path() {
  DWORD chars_required = ::GetTempPathA(0, nullptr);
  if (chars_required == 0) {
    DCHECK(false) << "Unable to query temp path size";
    return "";
  }
  std::string temp_path;
  temp_path.resize(chars_required - 1);
  if (::GetTempPathA(chars_required, &temp_path[0]) != temp_path.size()) {
    DCHECK(false) << "Unable to read temp path";
    return "";
  }
  return temp_path;
}

}  // namespace xrtl
