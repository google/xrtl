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

namespace xrtl {

absl::optional<std::string> Env::GetValue(absl::string_view key) {
  char key_buffer[255 + 1] = {0};
  if (key.size() >= ABSL_ARRAYSIZE(key_buffer)) {
    DCHECK(false) << "Attempting to access a really long env var: " << key;
    return absl::nullopt;
  }
  std::memcpy(key_buffer, key.data(), key.size());
  key_buffer[key.size()] = 0;
  char* value = std::getenv(key_buffer);
  if (!value) {
    return absl::nullopt;
  }
  return std::string(value);
}

std::string Env::QueryTempPath() {
  return Env::GetValueOrDefault("TMPDIR", "/tmp");
}

}  // namespace xrtl
