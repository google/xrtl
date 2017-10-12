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

#ifndef XRTL_GFX_SHADER_MODULE_H_
#define XRTL_GFX_SHADER_MODULE_H_

#include "xrtl/gfx/managed_object.h"

namespace xrtl {
namespace gfx {

// A module of shader code.
// Each module may contain multiple entry points that can be used by pipelines.
class ShaderModule : public ManagedObject {
 public:
  // Describes the format of shader module data.
  enum class DataFormat {
    // Standard SPIR-V.
    kSpirV,
  };

 protected:
  ShaderModule() = default;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_SHADER_MODULE_H_
