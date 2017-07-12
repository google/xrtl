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

#ifndef XRTL_GFX_ES3_ES3_PROGRAM_H_
#define XRTL_GFX_ES3_ES3_PROGRAM_H_

#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/array_view.h"
#include "xrtl/gfx/es3/es3_common.h"
#include "xrtl/gfx/es3/es3_platform_context.h"
#include "xrtl/gfx/es3/es3_shader.h"

namespace xrtl {
namespace gfx {
namespace es3 {

class ES3Program : public RefObject<ES3Program> {
 public:
  ES3Program(ref_ptr<ES3PlatformContext> platform_context,
             ArrayView<ref_ptr<ES3Shader>> shaders);
  ~ES3Program();

  GLuint program_id() const { return program_id_; }
  const std::vector<ref_ptr<ES3Shader>>& shaders() const { return shaders_; }

  // Program linking info log containing warnings and errors that accumulated
  // during linking.
  const std::string& info_log() const { return info_log_; }

  // Returns a mapping of binding slot binding index to GL binding index.
  const std::vector<GLuint>& set_binding_map(int set_index) const {
    return set_binding_maps_.set_bindings[set_index];
  }

  using PushConstantMemberPair =
      std::pair<const ES3Shader::PushConstantMember*, GLuint>;

  // Returns a list of all used push constant members across all shaders paired
  // with the GL uniform location of the member.
  const std::vector<PushConstantMemberPair>& push_constant_members() const {
    return push_constant_members_;
  }

  // Attempts to link the shaders into a program.
  // Returns false if the link failed. info_log can be used to get the
  // detailed error logs.
  bool Link();

 private:
  ref_ptr<ES3PlatformContext> platform_context_;
  std::vector<ref_ptr<ES3Shader>> shaders_;
  GLuint program_id_ = 0;

  std::string info_log_;

  ES3Shader::SetBindingMaps set_binding_maps_;
  std::vector<PushConstantMemberPair> push_constant_members_;
};

}  // namespace es3
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_ES3_ES3_PROGRAM_H_
