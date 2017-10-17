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

#ifndef XRTL_GFX_ES3_ES3_SHADER_MODULE_H_
#define XRTL_GFX_ES3_ES3_SHADER_MODULE_H_

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_queue_object.h"
#include "xrtl/gfx/es3/es3_shader.h"
#include "xrtl/gfx/shader_module.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3ShaderModule : public ShaderModule, public ES3QueueObject {
 public:
  explicit ES3ShaderModule(ES3ObjectLifetimeQueue* queue);
  ~ES3ShaderModule() override;

  void PrepareAllocation() override;

  // Registers a shader with the shader module.
  // The entry point of the shader must be unique within the module.
  // This is not thread safe and should only be used during initialization.
  void Register(ref_ptr<ES3Shader> shader);

  // Finds a shader by entry point name, if it exists.
  ref_ptr<ES3Shader> Lookup(absl::string_view entry_point) const;

 private:
  void Release() override;
  bool AllocateOnQueue() override;
  void DeallocateOnQueue() override;

  ES3ObjectLifetimeQueue* queue_;
  std::vector<ref_ptr<ES3Shader>> shaders_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_SHADER_MODULE_H_
