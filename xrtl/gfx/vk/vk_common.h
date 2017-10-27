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

#ifndef XRTL_GFX_VK_VK_COMMON_H_
#define XRTL_GFX_VK_VK_COMMON_H_

#if defined(XRTL_PLATFORM_WINDOWS)
// Ensure our windows.h is included instead of the one included by vulkan.
#include "xrtl/port/windows/base/windows.h"
#endif  // XRTL_PLATFORM_WINDOWS

// Primary vulkan headers.
// This must come after windows.h is included.
#include <vulkan/vulkan.h>  // NOLINT(build/include_order)

// Windows #define's this, which messes with our implementation.
#undef MemoryBarrier

#include "absl/strings/string_view.h"
#include "xrtl/base/logging.h"

namespace xrtl {
namespace gfx {
namespace vk {

// TODO(benvanik): use device limits resource_set_count.
constexpr int kMaxResourceSetCount = 4;

// Returns a string describing the given Vulkan result.
absl::string_view to_string(VkResult value);

}  // namespace vk
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_VK_VK_COMMON_H_
