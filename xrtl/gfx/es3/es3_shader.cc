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

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include "xrtl/base/debugging.h"
#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace es3 {

namespace {
using spirv_cross::SPIRType;
}  // namespace

ES3Shader::ES3Shader(ref_ptr<ES3PlatformContext> platform_context,
                     std::string entry_point)
    : platform_context_(std::move(platform_context)),
      entry_point_(std::move(entry_point)) {}

ES3Shader::~ES3Shader() {
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);
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
  auto context_lock =
      ES3PlatformContext::LockTransientContext(platform_context_);

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
      source_lengths.push_back(static_cast<GLint>(source.size()));
    }
    glShaderSource(shader_id_, static_cast<int>(source_strings.size()),
                   source_strings.data(), source_lengths.data());
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

  // Remove layout location specifiers on varyings as they are matched by name
  // in GL.
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

  // Record and then remove uniform binding sets/locations.
  // We'll later assign the explicit bindings in ApplyBindings per-program.
  auto ExtractUniformAssignment = [this, &compiler](
      const spirv_cross::Resource& resource, bool is_block) {
    int set = 0;
    int binding = 0;
    if (compiler.has_decoration(resource.id, spv::DecorationDescriptorSet)) {
      set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
      compiler.unset_decoration(resource.id, spv::DecorationDescriptorSet);
    }
    if (compiler.has_decoration(resource.id, spv::DecorationBinding)) {
      binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
      compiler.unset_decoration(resource.id, spv::DecorationBinding);
    }
    uniform_assignments_.push_back({resource.name, is_block, set, binding});
  };
  for (const auto& resource : shader_resources.uniform_buffers) {
    ExtractUniformAssignment(resource, true);
  }
  for (const auto& resource : shader_resources.storage_buffers) {
    ExtractUniformAssignment(resource, false);
  }
  for (const auto& resource : shader_resources.storage_images) {
    ExtractUniformAssignment(resource, false);
  }
  for (const auto& resource : shader_resources.sampled_images) {
    ExtractUniformAssignment(resource, false);
  }

  // Record and reflect push constant buffers. We emulate push constants with
  // normal nested GL struct uniform locations.
  for (const auto& resource : shader_resources.push_constant_buffers) {
    push_constant_block_name_ = resource.name;
    const SPIRType& type = compiler.get_type(resource.base_type_id);
    push_constant_members_.resize(type.member_types.size());
    for (int i = 0; i < type.member_types.size(); ++i) {
      PushConstantMember& member = push_constant_members_[i];
      member.member_name = compiler.get_member_name(type.self, i);
      // For now we only support primitives (float/vecN/matN/etc), and only
      // sizes we know about.
      const SPIRType& member_type = compiler.get_type(type.member_types[i]);
      member.member_offset = compiler.type_struct_member_offset(type, i);
      member.array_size = 1;  // TODO(benvanik): arrays.
      member.transpose = compiler.get_decoration(member_type.self,
                                                 spv::DecorationRowMajor) != 0;
      switch (member_type.basetype) {
        case SPIRType::Float:
          DCHECK_EQ(member_type.width, 32);
          if (member_type.columns > 1) {
            // Matrix types.
            static const GLenum kMatrixTypes[5][5] = {
                {GL_NONE, GL_NONE, GL_NONE, GL_NONE},  // 0 unused
                {GL_NONE, GL_NONE, GL_NONE, GL_NONE},  // 1 unused
                {GL_NONE, GL_NONE, GL_FLOAT_MAT2, GL_FLOAT_MAT2x3,
                 GL_FLOAT_MAT2x4},
                {GL_NONE, GL_NONE, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3,
                 GL_FLOAT_MAT3x4},
                {GL_NONE, GL_NONE, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3,
                 GL_FLOAT_MAT4},
            };
            DCHECK_LE(member_type.columns, 4);
            DCHECK_LE(member_type.vecsize, 4);
            member.member_type =
                kMatrixTypes[member_type.columns][member_type.vecsize];
          } else {
            // Scalar/vector types.
            static const GLenum kVectorTypes[5] = {
                GL_NONE, GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4};
            DCHECK_LE(member_type.vecsize, 4);
            member.member_type = kVectorTypes[member_type.vecsize];
          }
          break;
        default:
          // TODO(benvanik): richer type support.
          LOG(ERROR) << "Unsupported push constant member type";
          DCHECK(false);
          return false;
      }
    }
  }

  // Sort uniform assignments to make binding reservation easier in ES3Program.
  std::sort(uniform_assignments_.begin(), uniform_assignments_.end(),
            [](const UniformAssignment& a, const UniformAssignment& b) {
              if (a.set < b.set) {
                return true;
              } else if (a.set == b.set) {
                return a.binding < b.binding;
              } else {
                return false;
              }
            });

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

bool ES3Shader::ApplyBindings(GLuint program_id,
                              const SetBindingMaps& set_binding_maps) const {
  for (const UniformAssignment& assignment : uniform_assignments_) {
    GLuint gl_binding =
        set_binding_maps.set_bindings[assignment.set][assignment.binding];
    if (assignment.is_block) {
      GLint block_index =
          glGetUniformBlockIndex(program_id, assignment.uniform_name.c_str());
      if (block_index != -1) {
        glUniformBlockBinding(program_id, block_index, gl_binding);
      }
    } else {
      GLint uniform_location =
          glGetUniformLocation(program_id, assignment.uniform_name.c_str());
      if (uniform_location != -1) {
        glUniform1i(uniform_location, gl_binding);
      }
    }
  }
  return true;
}

GLint ES3Shader::QueryPushConstantLocation(
    GLuint program_id, const PushConstantMember& member) const {
  std::string full_name = push_constant_block_name_ + "." + member.member_name;
  return glGetUniformLocation(program_id, full_name.c_str());
}

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl
