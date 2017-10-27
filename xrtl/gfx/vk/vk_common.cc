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

#include "xrtl/gfx/vk/vk_common.h"

namespace xrtl {
namespace gfx {
namespace vk {

absl::string_view to_string(VkResult value) {
  switch (value) {
#define STR(r) \
  case r:      \
    return #r
    STR(VK_SUCCESS);
    STR(VK_NOT_READY);
    STR(VK_TIMEOUT);
    STR(VK_EVENT_SET);
    STR(VK_EVENT_RESET);
    STR(VK_INCOMPLETE);
    STR(VK_ERROR_OUT_OF_HOST_MEMORY);
    STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    STR(VK_ERROR_INITIALIZATION_FAILED);
    STR(VK_ERROR_DEVICE_LOST);
    STR(VK_ERROR_MEMORY_MAP_FAILED);
    STR(VK_ERROR_LAYER_NOT_PRESENT);
    STR(VK_ERROR_EXTENSION_NOT_PRESENT);
    STR(VK_ERROR_FEATURE_NOT_PRESENT);
    STR(VK_ERROR_INCOMPATIBLE_DRIVER);
    STR(VK_ERROR_TOO_MANY_OBJECTS);
    STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
    STR(VK_ERROR_FRAGMENTED_POOL);
    STR(VK_ERROR_SURFACE_LOST_KHR);
    STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
    STR(VK_SUBOPTIMAL_KHR);
    STR(VK_ERROR_OUT_OF_DATE_KHR);
    STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
    STR(VK_ERROR_VALIDATION_FAILED_EXT);
    STR(VK_ERROR_INVALID_SHADER_NV);
    STR(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
    STR(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);
    STR(VK_ERROR_NOT_PERMITTED_EXT);
#undef STR
    default:
      return "(unknown error)";
  }
}

}  // namespace vk
}  // namespace gfx
}  // namespace xrtl
