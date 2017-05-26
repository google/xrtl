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

#include "xrtl/gfx/es3/es3_shader.h"

#include <spirv_glsl.hpp>

#include <utility>
#include <vector>

#include "xrtl/base/debugging.h"
#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace es3 {

ES3Shader::ES3Shader(ref_ptr<ES3PlatformContext> platform_context,
                     std::string entry_point)
    : platform_context_(platform_context),
      entry_point_(std::move(entry_point)) {}

ES3Shader::~ES3Shader() {
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));
  if (shader_id_) {
    glDeleteShader(shader_id_);
  }
}

bool ES3Shader::CompileSource(GLenum shader_type,
                              ArrayView<const char*> sources) {
  std::vector<StringView> string_view_sources;
  string_view_sources.reserve(sources.size());
  for (const auto& source : sources) {
    string_view_sources.push_back(StringView(source));
  }
  return CompileSource(shader_type, string_view_sources);
}

bool ES3Shader::CompileSource(GLenum shader_type,
                              ArrayView<std::string> sources) {
  std::vector<StringView> string_view_sources;
  string_view_sources.reserve(sources.size());
  for (const auto& source : sources) {
    string_view_sources.push_back(StringView(source));
  }
  return CompileSource(shader_type, string_view_sources);
}

bool ES3Shader::CompileSource(GLenum shader_type,
                              ArrayView<StringView> sources) {
  WTF_SCOPE0("ES3Shader#CompileSource");
  ES3PlatformContext::ThreadLock context_lock(
      ES3PlatformContext::AcquireThreadContext(platform_context_));

  shader_type_ = shader_type;
  shader_id_ = glCreateShader(shader_type_);

  {
    // MESA has leaks in its shader compiler so we disable temporarily.
    debugging::LeakCheckDisabler leak_check_disabler;

    // Attach source now and start compilation.
    // On GL implementations without async compilation this will block the
    // thread.
    std::vector<const GLchar*> source_strings;
    source_strings.reserve(sources.size());
    std::vector<GLint> source_lengths;
    source_lengths.reserve(sources.size());
    for (const auto& source : sources) {
      source_strings.push_back(source.data());
      source_lengths.push_back(source.size());
    }
    glShaderSource(shader_id_, source_strings.size(), source_strings.data(),
                   source_lengths.data());
    glCompileShader(shader_id_);
  }

  GLint compile_status = 0;
  glGetShaderiv(shader_id_, GL_COMPILE_STATUS, &compile_status);

  // Note that even if compilation succeeds we may have warnings in the log.
  GLint info_log_length = 0;
  glGetShaderiv(shader_id_, GL_INFO_LOG_LENGTH, &info_log_length);
  info_log_.resize(info_log_length);
  glGetShaderInfoLog(shader_id_, info_log_length + 1, nullptr,
                     const_cast<GLchar*>(info_log_.data()));

  // TODO(benvanik): dump combined shader source.
  if (compile_status != GL_TRUE) {
    LOG(ERROR) << "Shader compilation failed: " << info_log_;
  } else if (info_log_.size() > 4) {
    VLOG(1) << "Shader compilation warnings: " << info_log_;
  }

  return compile_status == GL_TRUE;
}

bool ES3Shader::CompileSpirVBinary(const uint32_t* data, size_t data_length) {
  WTF_SCOPE0("ES3Shader#CompileSpirVBinary");

  // Prepare the translator with the input SPIR-V.
  spirv_cross::CompilerGLSL compiler(data, data_length);

  // Setup translation options.
  using Options = spirv_cross::CompilerGLSL::Options;
  Options options;
  options.version = 300;
  options.es = true;
  options.vulkan_semantics = false;
  options.vertex.fixup_clipspace = true;
  options.fragment.default_float_precision = Options::Highp;
  options.fragment.default_int_precision = Options::Highp;
  compiler.set_options(options);
  compiler.set_entry_point(entry_point_);

  // Determine shader type from execution mode.
  GLenum shader_type = GL_VERTEX_SHADER;
  switch (compiler.get_execution_model()) {
    case spv::ExecutionModelVertex:
      shader_type = GL_VERTEX_SHADER;
      break;
    case spv::ExecutionModelTessellationControl:
      shader_type = GL_TESS_CONTROL_SHADER;
      break;
    case spv::ExecutionModelTessellationEvaluation:
      shader_type = GL_TESS_EVALUATION_SHADER;
      break;
    case spv::ExecutionModelGeometry:
      shader_type = GL_GEOMETRY_SHADER;
      break;
    case spv::ExecutionModelFragment:
      shader_type = GL_FRAGMENT_SHADER;
      break;
    case spv::ExecutionModelGLCompute:
    case spv::ExecutionModelKernel:
      shader_type = GL_COMPUTE_SHADER;
      break;
    default:
      LOG(ERROR) << "Unknown SPIR-V execution model";
      return false;
  }

  // Perform some fixup for GLSL ES 300.
  // - remove layout location specifiers on varyings
  auto shader_resources = compiler.get_shader_resources();
  if (shader_type != GL_VERTEX_SHADER) {
    // Remove location specifiers from shader inputs (except vertex shaders).
    for (const auto& resource : shader_resources.stage_inputs) {
      if (compiler.has_decoration(resource.id, spv::DecorationLocation)) {
        compiler.unset_decoration(resource.id, spv::DecorationLocation);
      }
    }
  }
  if (shader_type != GL_FRAGMENT_SHADER) {
    // Remove location specifiers from shader outputs (except fragment shaders).
    for (const auto& resource : shader_resources.stage_outputs) {
      if (compiler.has_decoration(resource.id, spv::DecorationLocation)) {
        compiler.unset_decoration(resource.id, spv::DecorationLocation);
      }
    }
  }

  // Add common code and extension requirements.
  // TODO(benvanik): figure out what is required here.
  //   compiler.add_header_line("");
  //   compiler.require_extension("GL_KHR_foo");

  // Perform translation into GLSL.
  std::string translated_source = compiler.compile();
  if (translated_source.empty()) {
    LOG(ERROR) << "Failed to translate SPIR-V to GLSL; partial result:"
               << std::endl
               << compiler.get_partial_source() << std::endl;
    return false;
  }

  // TODO(benvanik): make a flag? useful now when testing translation.
  VLOG(2) << "Translated SPIR-V -> GLSL shader: " << std::endl
          << translated_source;

  // Attempt to compile GLSL to a native GL shader.
  return CompileSource(shader_type, ArrayView<std::string>{translated_source});
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
