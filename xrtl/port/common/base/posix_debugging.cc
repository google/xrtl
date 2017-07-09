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

#include "xrtl/base/debugging.h"

namespace xrtl {
namespace debugging {

void EnableDebugHeap() {
  // TODO(benvanik): something with tcmalloc?
}

bool is_console_attached() {
  // TODO(benvanik): console detection.
  return true;
}

bool AttachConsole() {
  // TODO(benvanik): console detection.
  return true;
}

bool is_debugger_attached() {
  // TODO(benvanik): debugger detection.
  return false;
}

}  // namespace debugging
}  // namespace xrtl
