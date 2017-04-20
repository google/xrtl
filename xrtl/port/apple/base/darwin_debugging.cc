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

#include <sys/sysctl.h>
#include <unistd.h>

#include "xrtl/base/debugging.h"

namespace xrtl {
namespace debugging {

bool is_console_attached() {
  // TODO(benvanik): console detection.
  return false;
}

bool AttachConsole() {
  // TODO(benvanik): console detection.
  return false;
}

bool is_debugger_attached() {
  // This works on both iOS and OSX.
  // https://developer.apple.com/library/mac/qa/qa1361/_index.html
  kinfo_proc info = {0};
  info.kp_proc.p_flag = 0;
  int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
  size_t size = sizeof(info);
  sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);
  return (info.kp_proc.p_flag & P_TRACED) != 0;
}

}  // namespace debugging
}  // namespace xrtl
