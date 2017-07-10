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

#include <mutex>

// WARNING: this massively pollutes the global namespace. Do not put this in .h.
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"

#include "xrtl/base/debugging.h"
#include "xrtl/base/tracing.h"

namespace xrtl {
namespace gfx {
namespace spirv {

namespace {

// TODO(benvanik): allow this to be cleaned up.
void InitializeGlslang() {
  static std::once_flag initialized_flag;
  std::call_once(initialized_flag, []() {
    WTF_SCOPE0("ShaderCompiler#InitializeGlslang");
    debugging::LeakCheckDisabler leak_check_disabler;
    glslang::InitializeProcess();
  });
}

// These define the limits used during validation. The actual limits when
// executing the shaders may be less than this, but this starting point allows
// us to get early errors in the common cases.
//
// TODO(benvanik): update this to the minspec across all APIs.
const TBuiltInResource kResourceLimits = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .limits = */
    {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

}  // namespace

ShaderCompiler::ShaderCompiler(SourceLanguage source_language,
                               ShaderStage shader_stage)
    : source_language_(source_language), shader_stage_(shader_stage) {}

ShaderCompiler::~ShaderCompiler() {
  // Program must be released first.
  program_.reset();
  shader_.reset();
}

void ShaderCompiler::AddSource(const std::string& file_name,
                               std::string source) {
  source_files_.push_back({file_name, std::move(source)});
}

void ShaderCompiler::AddSource(const std::string& file_name,
                               StringView source) {
  source_files_.push_back({file_name, std::string(source)});
}

bool ShaderCompiler::Compile(std::vector<uint32_t>* spirv_data) {
  WTF_SCOPE0("ShaderCompiler#Compile");
  spirv_data->clear();

  InitializeGlslang();

  EShLanguage sh_language = EShLangVertex;
  switch (shader_stage_) {
    case ShaderStage::kVertex:
      sh_language = EShLangVertex;
      break;
    case ShaderStage::kTessellationControl:
      sh_language = EShLangTessControl;
      break;
    case ShaderStage::kTessellationEvaluation:
      sh_language = EShLangTessEvaluation;
      break;
    case ShaderStage::kGeometry:
      sh_language = EShLangGeometry;
      break;
    case ShaderStage::kFragment:
      sh_language = EShLangFragment;
      break;
    case ShaderStage::kCompute:
      sh_language = EShLangCompute;
      break;
  }
  shader_ = make_unique<glslang::TShader>(sh_language);
  program_ = make_unique<glslang::TProgram>();
  compile_log_.clear();
  compile_log_verbose_.clear();

  // Populate sources.
  std::vector<const char*> source_strings;
  std::vector<int> source_lengths;
  std::vector<const char*> source_names;
  source_strings.reserve(source_files_.size());
  source_lengths.reserve(source_files_.size());
  source_names.reserve(source_files_.size());
  for (const auto& source_file : source_files_) {
    source_strings.push_back(source_file.source.c_str());
    source_lengths.push_back(static_cast<int>(source_file.source.size()));
    source_names.push_back(source_file.name.c_str());
  }
  shader_->setStringsWithLengthsAndNames(
      source_strings.data(), source_lengths.data(), source_names.data(),
      static_cast<int>(source_files_.size()));

  // TODO(benvanik): setPreamble to add some xrtl defines.

  // Specify entry point that will be kept. This will allow dead code removal
  // of other entry points.
  // TODO(benvanik): allow specifying input ('source') and output entry point.
  shader_->setEntryPoint("main");
  shader_->setSourceEntryPoint("main");

  EShMessages messages = EShMsgDefault;
  messages = static_cast<EShMessages>(messages | EShMsgSpvRules);
  // TODO(benvanik): do we want this? may cause issues with spirv-cross?
  messages = static_cast<EShMessages>(messages | EShMsgVulkanRules);

  // Parse the shader.
  bool did_parse;
  {
    // glslang leaks here... yep.
    debugging::LeakCheckDisabler leak_check_disabler;
    did_parse = shader_->parse(&kResourceLimits, 100, EEsProfile, false, true,
                               messages);
  }
  compile_log_ += shader_->getInfoLog();
  compile_log_verbose_ += shader_->getInfoDebugLog();
  if (!did_parse) {
    return false;
  }

  // Add the shader to the program for linking.
  // If we wanted to support multi-compilation unit linking this would be the
  // place to modify.
  program_->addShader(shader_.get());
  program_->mapIO();
  bool did_link = program_->link(messages);
  compile_log_ += program_->getInfoLog();
  compile_log_verbose_ += program_->getInfoDebugLog();
  if (!did_link) {
    return false;
  }

  // Get the glslang intermediate form for our stage.
  auto intermediate = program_->getIntermediate(sh_language);
  if (!intermediate) {
    LOG(ERROR) << "Unable to get shader stage intermediate form";
    return false;
  }

  // Convert the glslang form to SPIR-V.
  // NOTE: GlslangToSpv doesn't return an error code and the logger can have
  //       warnings even if conversion succeeds. So: no way to know if this
  //       worked.
  spv::SpvBuildLogger logger;
  glslang::GlslangToSpv(*intermediate, *spirv_data, &logger);
  compile_log_ += logger.getAllMessages();

  return true;
}

}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl
