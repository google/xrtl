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

#include "xrtl/gfx/es3/es3_shader_module.h"

#include <utility>

namespace xrtl {
namespace gfx {
namespace es3 {

ES3ShaderModule::ES3ShaderModule(ES3ObjectLifetimeQueue* queue)
    : queue_(queue) {
}

ES3ShaderModule::~ES3ShaderModule() = default;

void ES3ShaderModule::PrepareAllocation() {
  queue_->EnqueueObjectAllocation(this);
}

void ES3ShaderModule::Release() { queue_->EnqueueObjectDeallocation(this); }

bool ES3ShaderModule::AllocateOnQueue() { return true; }

void ES3ShaderModule::DeallocateOnQueue() { shaders_.clear(); }

void ES3ShaderModule::Register(ref_ptr<ES3Shader> shader) {
  shaders_.push_back(shader);
}

ref_ptr<ES3Shader> ES3ShaderModule::Lookup(
    absl::string_view entry_point) const {
  for (const auto& shader : shaders_) {
    if (entry_point == shader->entry_point()) {
      return shader;
    }
  }
  return nullptr;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
