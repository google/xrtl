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

#ifndef XRTL_PORT_COMMON_BASE_MACH_SYSTEM_CLOCK_H_
#define XRTL_PORT_COMMON_BASE_MACH_SYSTEM_CLOCK_H_

#include <memory>

#include "xrtl/base/system_clock.h"

namespace xrtl {

// Creates a system clock backed by the Mach APIs.
std::unique_ptr<SystemClock> CreateMachSystemClock();

}  // namespace xrtl

#endif  // XRTL_PORT_COMMON_BASE_MACH_SYSTEM_CLOCK_H_
