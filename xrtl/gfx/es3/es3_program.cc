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

#include "xrtl/gfx/es3/es3_program.h"

#include <utility>

#include "xrtl/base/tracing.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Program::ES3Program(absl::Span<const ref_ptr<ES3Shader>> shaders)
    : shaders_(shaders.begin(), shaders.end()) {}

ES3Program::~ES3Program() {
  ES3PlatformContext::CheckHasContextLock();
  if (program_id_) {
    glDeleteProgram(program_id_);
    program_id_ = 0;
  }
}

bool ES3Program::Link() {
  WTF_SCOPE0("ES3Program#Link");
  ES3PlatformContext::CheckHasContextLock();

  DCHECK_EQ(program_id_, 0) << "Attemping to re-link a program";

  program_id_ = glCreateProgram();

  for (const auto& shader : shaders_) {
    // Compile the shader (may be a no-op if already compiled).
    if (!shader->Validate()) {
      LOG(ERROR) << "Shader validation failed";
      return false;
    }

    glAttachShader(program_id_, shader->shader_id());
  }

  glLinkProgram(program_id_);

  // TODO(benvanik): validate?

  GLint link_status = 0;
  glGetProgramiv(program_id_, GL_LINK_STATUS, &link_status);

  GLint info_log_length = 0;
  glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &info_log_length);
  info_log_.resize(info_log_length);
  glGetProgramInfoLog(program_id_, info_log_length + 1, nullptr,
                      const_cast<GLchar*>(info_log_.data()));

  // TODO(benvanik): dump combined shader source.
  if (link_status != GL_TRUE) {
    LOG(ERROR) << "Program linking failed: " << info_log_;
    return false;
  }
  if (info_log_.size() > 4) {
    VLOG(1) << "Program linking warnings: " << info_log_;
  }

  // Perform binding allocation across the shaders.
  GLuint next_binding_index = 0;
  for (auto& shader : shaders_) {
    // Assignments must be sorted by set+binding.
    for (const auto& assignment : shader->uniform_assignments()) {
      auto& set_bindings = set_binding_maps_.set_bindings[assignment.set];
      if (set_bindings.size() <= assignment.binding) {
        set_bindings.resize(static_cast<size_t>(assignment.binding) + 1);
        set_bindings[assignment.binding] = next_binding_index++;
      }
    }
  }

  // Initialize shader bindings.
  glUseProgram(program_id_);
  for (const auto& shader : shaders_) {
    if (!shader->ApplyBindings(program_id_, set_binding_maps_)) {
      LOG(ERROR) << "Failed to apply shader uniform bindings";
      return false;
    }
  }
  glUseProgram(0);

  // Merge shader push constant locations (as they should be shared, though
  // the set of valid members may differ for each).
  uint64_t location_set = 0;
  for (const auto& shader : shaders_) {
    for (const auto& member : shader->push_constant_members()) {
      GLint uniform_location =
          shader->QueryPushConstantLocation(program_id_, member);
      if (uniform_location != -1) {
        if ((location_set & (1 << uniform_location)) == 0) {
          location_set |= (1 << uniform_location);
          push_constant_members_.push_back({&member, uniform_location});
        }
      }
    }
  }

  return link_status == GL_TRUE;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
