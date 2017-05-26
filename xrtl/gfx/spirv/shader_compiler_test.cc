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

#include "xrtl/gfx/spirv/shader_compiler.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace gfx {
namespace spirv {
namespace {

// Tests that the compiler can be created and deleted with no leaks.
TEST(ShaderCompilerTest, Unused) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  EXPECT_EQ(ShaderCompiler::SourceLanguage::kGlsl, compiler.source_language());
  EXPECT_EQ(ShaderCompiler::ShaderStage::kVertex, compiler.shader_stage());
  EXPECT_TRUE(compiler.compile_log().empty());
  EXPECT_TRUE(compiler.compile_log_verbose().empty());
}

// Tests compiling with no inputs. It should pass (but provide no results).
TEST(ShaderCompilerTest, Empty) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  std::vector<uint32_t> spirv_data;
  // To test that the target data is cleared even on empty.
  spirv_data.push_back(1);
  EXPECT_TRUE(compiler.Compile(&spirv_data));
  EXPECT_TRUE(spirv_data.empty());
  EXPECT_TRUE(compiler.compile_log().empty());
}

// Tests compiling with an input that has a warning.
TEST(ShaderCompilerTest, SourceWarning) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  compiler.AddSource("inline", R"""(#version 310 es
#extension foo : warn
void main() {}
)""");
  std::vector<uint32_t> spirv_data;
  EXPECT_TRUE(compiler.Compile(&spirv_data));
  EXPECT_FALSE(spirv_data.empty());
  EXPECT_FALSE(compiler.compile_log().empty());
  EXPECT_NE(std::string::npos,
            compiler.compile_log().find("WARNING: inline:2: "));
}

// Tests compiling with an input that has an error.
TEST(ShaderCompilerTest, SourceError) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  compiler.AddSource("inline", R"""(#version 310 es
#extension foo : require
void main() {}
)""");
  std::vector<uint32_t> spirv_data;
  // To test that the target data is cleared even on error.
  spirv_data.push_back(1);
  EXPECT_FALSE(compiler.Compile(&spirv_data));
  EXPECT_TRUE(spirv_data.empty());
  EXPECT_FALSE(compiler.compile_log().empty());
  EXPECT_NE(std::string::npos,
            compiler.compile_log().find("ERROR: inline:2: "));
}

// Tests compiling valid input.
TEST(ShaderCompilerTest, ValidSource) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  compiler.AddSource("inline", R"""(#version 310 es
void main() {}
)""");
  std::vector<uint32_t> spirv_data;
  EXPECT_TRUE(compiler.Compile(&spirv_data));
  EXPECT_EQ(42, spirv_data.size());  // Probably OK.
}

// Tests compiling multiple inputs.
TEST(ShaderCompilerTest, MultipleSources) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  compiler.AddSource("#version 310 es\n");
  compiler.AddSource("inline", "void main() {}");
  std::vector<uint32_t> spirv_data;
  EXPECT_TRUE(compiler.Compile(&spirv_data));
  EXPECT_EQ(42, spirv_data.size());  // Probably OK.
}

// Tests reusing the same compiler instance.
TEST(ShaderCompilerTest, Reuse) {
  ShaderCompiler compiler(ShaderCompiler::SourceLanguage::kGlsl,
                          ShaderCompiler::ShaderStage::kVertex);
  compiler.AddSource("#version 310 es\n");
  compiler.AddSource("inline1", "void main() {}");
  std::vector<uint32_t> spirv_data;
  EXPECT_TRUE(compiler.Compile(&spirv_data));
  EXPECT_EQ(42, spirv_data.size());  // Probably OK.
  spirv_data.clear();
  EXPECT_TRUE(compiler.Compile(&spirv_data));
  EXPECT_EQ(42, spirv_data.size());  // Probably OK.
}

}  // namespace
}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl
