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

#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Program::ES3Program(ref_ptr<ES3PlatformContext> platform_context,
                       ArrayView<ref_ptr<ES3Shader>> shaders)
    : platform_context_(platform_context),
      shaders_(std::vector<ref_ptr<ES3Shader>>(shaders)) {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));

  program_id_ = glCreateProgram();

  for (const auto& shader : shaders_) {
    glAttachShader(program_id_, shader->shader_id());
  }
}

ES3Program::~ES3Program() {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));
  if (program_id_) {
    glDeleteProgram(program_id_);
  }
}

bool ES3Program::Link() {
  WTF_SCOPE0("ES3Program#Link");
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));

  glLinkProgram(program_id_);

  // TODO(benvanik): validate?

  // TODO(benvanik): glUseProgram to force first usage and *really* link?

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
  } else if (info_log_.size() > 4) {
    VLOG(1) << "Program linking warnings: " << info_log_;
  }

  return link_status == GL_TRUE;
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
