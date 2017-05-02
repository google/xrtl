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

#include "xrtl/gfx/context_factory.h"

#include "xrtl/base/flags.h"
#include "xrtl/base/logging.h"

DEFINE_string(gfx,
              "Graphics backend used for rendering and compute: "
              "[nop, es3, metal, vulkan]",
              "");

namespace xrtl {
namespace gfx {

// Creates a context factory with the given backend name.
// Pass empty string to get the default platform backend factory.
// Returns nullptr if the backend is not compiled in or supported on the
// current platform.
//
// Valid values:
//   '': platform default, never nop.
//   'nop': no-op (null) backend; performs no rendering, testing-only.
//   'es3': OpenGL ES 3.X (Android/Emscripten/Linux/iOS/MacOS/Windows)
//   'metal': Metal (iOS/MacOS only)
//   'vulkan': Vulkan (Android/Linux/Windows)
ref_ptr<ContextFactory> ContextFactory::Create(std::string name) {
  // Use flag if no name is provided.
  if (name.empty()) {
    name = FLAGS_gfx;
  }

  // Only allow nop when specified explicitly.
  if (name == "nop") {
    return nullptr;  // TODO(benvanik): nop.
  }

  // Build a list of available context types sorted by platform priority.
  std::vector<std::string> available_types;
  // TODO(benvanik): #if switch based on defines.

  // Find the desired type.
  std::string desired_type;
  if (name.empty()) {
    // Pick the first available type.
    desired_type = available_types.front();
  } else {
    for (auto available_type : available_types) {
      if (available_type == name) {
        desired_type = name;
        break;
      }
    }
  }
  if (desired_type.empty()) {
    // Could not find any type to use.
    return nullptr;
  }

  // Create the type.
  // TODO(benvanik): #if switch based on defines.

  return nullptr;
}

}  // namespace gfx
}  // namespace xrtl
