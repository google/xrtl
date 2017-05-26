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

#ifndef XRTL_GFX_SPIRV_SHADER_COMPILER_H_
#define XRTL_GFX_SPIRV_SHADER_COMPILER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "xrtl/base/string_view.h"

// Forward declarations as glslang headers are terrible.
namespace glslang {
class TShader;
class TProgram;
}  // namespace glslang

namespace xrtl {
namespace gfx {
namespace spirv {

// Shader compiler interface.
// This accepts shader source in various high-level languages (such as GLSL) and
// produces unoptimized SPIR-V bytecode. The results can be used directly by the
// graphics system or passed to a SpirVOptimizer for further optimization.
//
// Upon first use a compiler may initialize some process-local state. There is
// unfortunately no way to release this memory until the process exits. Multiple
// compiler instances may exist at a time and be used concurrently from multiple
// threads. A single compiler instance is not thread safe and must only be used
// from one thread at a time.
class ShaderCompiler {
 public:
  // Defines the source language provided to the compiler.
  enum class SourceLanguage {
    kGlsl,
    kHlsl,
  };

  // Defines the stage of the shader source.
  enum class ShaderStage {
    kVertex,
    kTessellationControl,
    kTessellationEvaluation,
    kGeometry,
    kFragment,
    kCompute,
  };

  ShaderCompiler(SourceLanguage source_language, ShaderStage shader_stage);
  ~ShaderCompiler();

  // Source language type that will be provided in AddSource calls.
  SourceLanguage source_language() const { return source_language_; }
  // Shader execution stage being compiled.
  ShaderStage shader_stage() const { return shader_stage_; }

  // Adds source code to compile.
  // It will be treated as if it came from a file named file_name.
  // Multiple sources will be concatenated. Add newlines to ensure that source
  // is properly attributed to the source file.
  void AddSource(const std::string& file_name, std::string source);
  void AddSource(const std::string& file_name, StringView source);
  void AddSource(const std::string& file_name, const char* source) {
    return AddSource(file_name, StringView{source});
  }

  // Adds source code to compile.
  // It will be appended to the other source provided with the default name.
  // Multiple sources will be concatenated. Add newlines to ensure that source
  // is properly attributed to the source file.
  void AddSource(std::string source) {
    return AddSource("", std::move(source));
  }
  void AddSource(StringView source) { return AddSource("", source); }
  void AddSource(const char* source) { return AddSource("", source); }

  // TODO(benvanik): preprocessor APIs for evaluation and #include support.

  // TODO(benvanik): linking APIs to enable cross-shader optimization?

  // Compiles the shader and produces SPIR-V bytecode.
  // Populates the provided vector with SPIR-V words upon success.
  // Returns true if the compilation suceeded. compile_log may contain warnings
  // even if compilation succeeds.
  bool Compile(std::vector<uint32_t>* spirv_data);

  // TODO(benvanik): rich error logging support.
  // Returns a log of compilation errors and warnings after Compile has been
  // called.
  std::string compile_log() const { return compile_log_; }
  // Returns a log with debug verbosity after Compile has been called.
  std::string compile_log_verbose() const { return compile_log_verbose_; }

  // TODO(benvanik): generate proto data (for reflection).

 private:
  SourceLanguage source_language_;
  ShaderStage shader_stage_;

  // TODO(benvanik): avoid copy when input is known constant.
  struct SourceFile {
    std::string name;
    std::string source;
  };
  std::vector<SourceFile> source_files_;

  std::string compile_log_;
  std::string compile_log_verbose_;

  std::unique_ptr<glslang::TShader> shader_;
  std::unique_ptr<glslang::TProgram> program_;
};

}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_SPIRV_SHADER_COMPILER_H_
