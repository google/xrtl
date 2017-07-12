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

#ifndef XRTL_GFX_ES3_ES3_SHADER_H_
#define XRTL_GFX_ES3_ES3_SHADER_H_

#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/base/string_view.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3Shader : public RefObject<ES3Shader> {
 public:
  // Defines a uniform assignment within the shader.
  struct UniformAssignment {
    // Uniform name that can be used with GL calls.
    std::string uniform_name;
    // True if it's a uniform block binding (instead of a normal uniform).
    bool is_block;
    // layout(set=X) value from shader.
    int set;
    // layout(binding=X) value from shader.
    int binding;
  };

  // Defines a push constant struct member as reflected from the shader.
  struct PushConstantMember {
    // The local name of the push constant member.
    std::string member_name;
    // Offset of the member in the struct, in bytes.
    int member_offset;
    // The GL type of the push constant member (such as GL_FLOAT_VEC3).
    GLenum member_type;
    // True when the member is a matrix and should be transposed.
    bool transpose;
    // Array size, in elements. Will be 1 if not an array.
    int array_size;
  };

  ES3Shader(ref_ptr<ES3PlatformContext> platform_context,
            std::string entry_point);
  ~ES3Shader();

  const std::string& entry_point() const { return entry_point_; }
  GLenum shader_type() const { return shader_type_; }
  GLuint shader_id() const { return shader_id_; }

  // Returns a list of all uniform assignments.
  // The assignments are sorted by set+binding.
  const std::vector<UniformAssignment>& uniform_assignments() const {
    return uniform_assignments_;
  }
  // Returns a list of all push constant members.
  const std::vector<PushConstantMember>& push_constant_members() const {
    return push_constant_members_;
  }

  // Shader compilation info log containing warnings and errors that accumulated
  // during compilation.
  const std::string& info_log() const { return info_log_; }

  // Attempts to compile the given GLSL source code into a shader.
  // Returns false if the compilation fails.
  bool CompileSource(GLenum shader_type, ArrayView<const char*> sources);
  bool CompileSource(GLenum shader_type, ArrayView<std::string> sources);
  bool CompileSource(GLenum shader_type, ArrayView<StringView> sources);

  // Attempts to translate a SPIR-V binary into GLSL and compile that.
  // Returns false if the binary cannot be translated or if compilation fails.
  bool CompileSpirVBinary(const uint32_t* data, size_t data_length);

  struct SetBindingMaps {
    std::vector<GLuint> set_bindings[kMaxResourceSetCount];
  };
  // Initializes all bindings for the currently bound program.
  // This must be called after a program using this shader is linked.
  bool ApplyBindings(GLuint program_id,
                     const SetBindingMaps& set_binding_maps) const;

  // Queries the uniform location of a push constant member in the given
  // program.
  GLint QueryPushConstantLocation(GLuint program_id,
                                  const PushConstantMember& member) const;

 private:
  ref_ptr<ES3PlatformContext> platform_context_;
  std::string entry_point_;
  GLenum shader_type_ = GL_VERTEX_SHADER;
  GLuint shader_id_ = 0;

  std::string info_log_;

  std::vector<UniformAssignment> uniform_assignments_;
  std::string push_constant_block_name_;
  std::vector<PushConstantMember> push_constant_members_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_SHADER_H_
