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

#include <spirv-tools/libspirv.hpp>

#include <algorithm>
#include <cerrno>
#include <fstream>
#include <string>

#include "xrtl/base/cli_main.h"
#include "xrtl/base/flags.h"
#include "xrtl/base/logging.h"
#include "xrtl/gfx/spirv/shader_compiler.h"
#include "xrtl/gfx/spirv/spirv_optimizer.h"

DEFINE_string(defines, "", "Comma separated list of values to #define.");

DEFINE_string(source_language, "glsl", "Source language type; [glsl, hlsl].");
DEFINE_string(source_version, "310 es",
              "#version header to interpret source with.");
DEFINE_string(shader_stage, "",
              "Shader stage (vert, frag, etc). If omitted it will be inferred "
              "from the input path.");

DEFINE_string(output_spirv, "", "Output SPIR-V binary file path.");
DEFINE_string(output_spirv_disasm, "", "Output SPIR-V disassembly file path.");

DEFINE_string(output_spirv_h, "", "Output SPIR-V C++ header file path.");
DEFINE_string(output_spirv_cc, "", "Output SPIR-V C++ source file path.");
DEFINE_string(output_namespace, "",
              "C++ namespace to embed the variable in (like 'foo::bar').");
DEFINE_string(output_variable, "kBytecode", "C++ variable name for the data.");

// TODO(benvanik): output_proto option.

DEFINE_bool(optimize, true, "Perform optimizations on the SPIR-V bytecode.");
DEFINE_bool(strip_debug_info, false, "True to strip all debug info.");

namespace xrtl {
namespace gfx {
namespace spirv {

namespace {

// Attempts to infer the shader stage from the given (dotless) file extension.
// Returns true if the shader stage was recognized.
bool InferShaderStage(const std::string& extension,
                      ShaderCompiler::ShaderStage* out_shader_stage) {
  if (extension == "vert") {
    *out_shader_stage = ShaderCompiler::ShaderStage::kVertex;
    return true;
  } else if (extension == "tesc") {
    *out_shader_stage = ShaderCompiler::ShaderStage::kTessellationControl;
    return true;
  } else if (extension == "tese") {
    *out_shader_stage = ShaderCompiler::ShaderStage::kTessellationEvaluation;
    return true;
  } else if (extension == "geom") {
    *out_shader_stage = ShaderCompiler::ShaderStage::kGeometry;
    return true;
  } else if (extension == "frag") {
    *out_shader_stage = ShaderCompiler::ShaderStage::kFragment;
    return true;
  } else if (extension == "comp") {
    *out_shader_stage = ShaderCompiler::ShaderStage::kCompute;
    return true;
  } else {
    *out_shader_stage = ShaderCompiler::ShaderStage::kVertex;
    return false;
  }
}

}  // namespace

// Shader compiler/optimizer/etc.
// Designed to be run offline (either from bazel rules or manually).
//
// Examples:
//
// # Compile a GLSL shader into optimized SPIRV bytecode:
// $ shader_tool --input_glsl=input.frag --output_spirv=output.frag.spv
//
// # Generate a C++ header/source with the embedded SPIRV bytecode:
// $ shader_tool --input_glsl=input.frag \
//               --output_spirv_h=output_frag.h \
//               --output_spirv_cc=output_frag.cc
int ShaderToolMain(int argc, char** argv) {
  // Convert input paths into strings we can easily handle.
  std::vector<std::string> input_paths;
  input_paths.reserve(argc - 1);
  for (int i = 1; i < argc; ++i) {
    input_paths.push_back(std::string(argv[i]));
  }
  if (input_paths.empty()) {
    LOG(ERROR) << "One or more input files required.";
    return 1;
  }

  ShaderCompiler::SourceLanguage source_language;
  if (FLAGS_source_language == "glsl") {
    source_language = ShaderCompiler::SourceLanguage::kGlsl;
    VLOG(1) << "Parsing inputs as GLSL";
  } else if (FLAGS_source_language == "hlsl") {
    source_language = ShaderCompiler::SourceLanguage::kHlsl;
    VLOG(1) << "Parsing inputs as HLSL";
  } else {
    LOG(ERROR) << "Unknown source language: " << FLAGS_source_language;
    return 1;
  }

  ShaderCompiler::ShaderStage shader_stage =
      ShaderCompiler::ShaderStage::kVertex;
  if (!FLAGS_shader_stage.empty()) {
    // Take user-specified shader stage instead of scanning file paths.
    if (!InferShaderStage(FLAGS_shader_stage, &shader_stage)) {
      LOG(ERROR) << "Unrecognized shader stage '" << FLAGS_shader_stage << "'.";
      return 1;
    }
  } else {
    // Scan all input paths and see if there's an extension we understand.
    bool found_any = false;
    for (const std::string& input_path : input_paths) {
      size_t last_dot = input_path.rfind('.');
      if (last_dot == std::string::npos) {
        continue;
      }
      std::string extension = input_path.substr(last_dot + 1);
      if (extension.empty()) {
        continue;
      }
      if (InferShaderStage(extension, &shader_stage)) {
        // Found one!
        VLOG(1) << "Inferred shader type as " << extension;
        found_any = true;
        break;
      }
    }
    if (!found_any) {
      LOG(ERROR) << "No shader stage could be inferred from input file paths. "
                    "Pass --shader_stage.";
      return 1;
    }
  }

  ShaderCompiler shader_compiler(source_language, shader_stage);

  // Add source version, as it must be the first line.
  shader_compiler.AddSource(std::string("#version ") + FLAGS_source_version +
                            "\n");

  // Split up input defines.
  std::vector<std::string> defines;
  if (!FLAGS_defines.empty()) {
    size_t current_sep = 0;
    size_t next_sep = FLAGS_defines.find(',');
    defines.push_back(FLAGS_defines.substr(current_sep, next_sep));
    current_sep = next_sep;
    while (next_sep != std::string::npos) {
      next_sep = FLAGS_defines.find(',', current_sep + 1);
      defines.push_back(
          FLAGS_defines.substr(current_sep + 1, next_sep - current_sep - 1));
      current_sep = next_sep;
    }
  }
  for (const std::string& define : defines) {
    VLOG(1) << "Prepending: #define " << define;
    std::string define_line = "#define " + define + "\n";
    shader_compiler.AddSource(define_line);
  }

  // Setup shader compiler inputs.
  for (const std::string& input_path : input_paths) {
    VLOG(1) << "Adding input file: " << input_path;

    // Load file contents.
    std::ifstream input_file(input_path, std::ios::in);
    if (!input_file) {
      LOG(ERROR) << "Failed to read input file " << input_path << ": " << errno;
      return 1;
    }
    input_file.seekg(0, std::ios::end);
    std::string input_contents;
    input_contents.resize(input_file.tellg());
    DCHECK(!input_contents.empty());
    input_file.seekg(0, std::ios::beg);
    input_file.read(&input_contents[0], input_contents.size());
    input_file.close();

    // Strip trailing NUL characters that can sometimes creep in.
    while (input_contents[input_contents.size() - 1] == 0) {
      input_contents.resize(input_contents.size() - 1);
    }

    // Add with file name so we'll get good error messages.
    shader_compiler.AddSource(input_path, input_contents);
  }

  // Perform compilation.
  VLOG(1) << "Compiling...";
  std::vector<uint32_t> spirv_data;
  bool did_compile = shader_compiler.Compile(&spirv_data);
  if (!shader_compiler.compile_log().empty()) {
    if (!did_compile) {
      // Errors!
      LOG(ERROR) << "Compilation failed:" << std::endl
                 << shader_compiler.compile_log();
    } else {
      LOG(WARNING) << "Compilation succeeded with warnings:" << std::endl
                   << shader_compiler.compile_log();
    }
  } else if (did_compile) {
    VLOG(1) << "Compilation successful!";
  } else {
    LOG(ERROR) << "Compilation failed!";
  }
  if (!shader_compiler.compile_log_verbose().empty()) {
    VLOG(1) << "Verbose log:" << std::endl
            << shader_compiler.compile_log_verbose();
  }
  if (!did_compile) {
    return 1;
  }

  // TODO(benvanik): ensure we want this environment.
  spvtools::SpirvTools spirv_tools(SPV_ENV_UNIVERSAL_1_0);
  spirv_tools.SetMessageConsumer(
      [](spv_message_level_t level, const char* source,
         const spv_position_t& position, const char* message) {
        int severity = 0;
        switch (level) {
          case SPV_MSG_FATAL:
          case SPV_MSG_INTERNAL_ERROR:
            severity = FATAL;
            break;
          case SPV_MSG_ERROR:
            severity = ERROR;
            break;
          case SPV_MSG_WARNING:
            severity = WARNING;
            break;
          default:
          case SPV_MSG_INFO:
          case SPV_MSG_DEBUG:
            severity = INFO;
            break;
        }
        LogString(source, position.line, severity, message);
      });

  // Validate SPIR-V bytecode.
  if (!spirv_tools.Validate(spirv_data)) {
    LOG(ERROR) << "Compiled (non-optimized) SPIR-V failed validation";
    return 1;
  }

  // Perform optimizations on the SPIR-V.
  // TODO(benvanik): add optimization levels.
  SpirVOptimizer::Options options;
  options.strip_debug_info = FLAGS_strip_debug_info;
  options.aggressive = FLAGS_optimize;
  options.remap_ids = true;
  SpirVOptimizer optimizer(options);

  std::vector<uint32_t> optimized_spirv_data;
  VLOG(1) << "Optimizing...";
  bool did_optimize = optimizer.Optimize(spirv_data, &optimized_spirv_data);
  if (!did_optimize) {
    LOG(ERROR) << "Optimization failed!";
    return 1;
  }
  VLOG(1) << "Optimization successful; was " << spirv_data.size() << "dw, now "
          << optimized_spirv_data.size() << "dw";
  std::swap(optimized_spirv_data, spirv_data);

  // Validate SPIR-V bytecode post-optimization.
  if (!spirv_tools.Validate(spirv_data)) {
    LOG(ERROR) << "Compiled and optimized SPIR-V failed validation";
    return 1;
  }

  // Write SPIR-V bytecode.
  if (!FLAGS_output_spirv.empty()) {
    VLOG(1) << "Writing SPIR-V bytecode to " << FLAGS_output_spirv;
    std::ofstream output_file;
    output_file.open(FLAGS_output_spirv,
                     std::ios::out | std::ios::binary | std::ios::trunc);
    if (!output_file.is_open()) {
      LOG(ERROR) << "Unable to open SPIR-V output file " << FLAGS_output_spirv;
      return 1;
    }
    output_file.write(reinterpret_cast<const char*>(spirv_data.data()),
                      spirv_data.size() * 4);
    output_file.close();
  }

  // Disassemble SPIR-V bytecode into text, as we write it as comments and such.
  int disasm_options = SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                       SPV_BINARY_TO_TEXT_OPTION_INDENT |
                       SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
  std::string disasm_string;
  if (!spirv_tools.Disassemble(spirv_data, &disasm_string, disasm_options)) {
    LOG(ERROR) << "Failed to disassemble SPIR-V";
    return 1;
  }

  // Write disassembly if asked.
  // TODO(benvanik): also write before optimization?
  if (!FLAGS_output_spirv_disasm.empty()) {
    VLOG(1) << "Writing SPIR-V disassembly to " << FLAGS_output_spirv_disasm;
    std::ofstream output_file;
    output_file.open(FLAGS_output_spirv_disasm,
                     std::ios::out | std::ios::trunc);
    if (!output_file.is_open()) {
      LOG(ERROR) << "Unable to open SPIR-V disassembly output file "
                 << FLAGS_output_spirv_disasm;
      return 1;
    }
    output_file << disasm_string;
    output_file.close();
  }

  // Generate C++ code for the SPIR-V.
  if (!FLAGS_output_spirv_h.empty() && !FLAGS_output_spirv_cc.empty()) {
    VLOG(1) << "Writing SPIR-V C++ files to " << FLAGS_output_spirv_h << " + "
            << FLAGS_output_spirv_cc;

    // Extract the name of the header file (like 'foo.h').
    std::string header_file_name = FLAGS_output_spirv_h;
    std::replace(header_file_name.begin(), header_file_name.end(), '\\', '/');
    size_t last_slash = header_file_name.rfind('/');
    if (last_slash != std::string::npos) {
      header_file_name = header_file_name.substr(last_slash + 1);
    }

    // Split up namespace into its parts.
    std::vector<std::string> namespace_parts;
    if (!FLAGS_output_namespace.empty()) {
      size_t current_sep = 0;
      size_t next_sep = FLAGS_output_namespace.find("::");
      namespace_parts.push_back(
          FLAGS_output_namespace.substr(current_sep, next_sep));
      current_sep = next_sep;
      while (next_sep != std::string::npos) {
        next_sep = FLAGS_output_namespace.find("::", current_sep + 2);
        namespace_parts.push_back(FLAGS_output_namespace.substr(
            current_sep + 2, next_sep - current_sep - 2));
        current_sep = next_sep;
      }
    }

    // Derive a header guard based on namespace and variable name.
    std::string header_guard;
    for (const std::string& namespace_part : namespace_parts) {
      header_guard += namespace_part + "_";
    }
    header_guard += header_file_name + "_";
    std::replace(header_guard.begin(), header_guard.end(), '.', '_');
    std::replace(header_guard.begin(), header_guard.end(), '-', '_');
    std::transform(header_guard.begin(), header_guard.end(),
                   header_guard.begin(), toupper);

    std::ofstream output_file_cc;
    output_file_cc.open(FLAGS_output_spirv_cc, std::ios::out | std::ios::trunc);
    if (!output_file_cc.is_open()) {
      LOG(ERROR) << "Unable to open output C++ source file "
                 << FLAGS_output_spirv_cc;
      return 1;
    }

    std::ofstream output_file_h;
    output_file_h.open(FLAGS_output_spirv_h, std::ios::out | std::ios::trunc);
    if (!output_file_h.is_open()) {
      LOG(ERROR) << "Unable to open output C++ header file "
                 << FLAGS_output_spirv_h;
      return 1;
    }
    output_file_h << "// DO NOT MODIFY: autogenerated by shader_tool."
                  << std::endl
                  << std::endl
                  << "#ifndef " << header_guard << std::endl
                  << "#define " << header_guard << std::endl
                  << std::endl
                  << "#include <cstdint>" << std::endl
                  << std::endl;
    for (int i = 0; i < namespace_parts.size(); ++i) {
      output_file_h << "namespace " << namespace_parts[i] << " {" << std::endl;
    }
    if (!namespace_parts.empty()) {
      output_file_h << std::endl;
    }
    output_file_h << "extern const uint32_t " << FLAGS_output_variable << "["
                  << spirv_data.size() << "];" << std::endl;
    if (!namespace_parts.empty()) {
      output_file_h << std::endl;
    }
    for (int i = namespace_parts.size() - 1; i >= 0; --i) {
      output_file_h << "}  // namespace " << namespace_parts[i] << std::endl;
    }
    output_file_h << std::endl << "#endif  // " << header_guard << std::endl;
    output_file_h.close();

    output_file_cc << "// DO NOT MODIFY: autogenerated by shader_tool."
                   << std::endl
                   << std::endl
                   << "#include \"" << header_file_name << "\"" << std::endl
                   << std::endl
                   << "/*" << std::endl
                   << disasm_string << std::endl
                   << "*/" << std::endl
                   << std::endl;
    for (int i = 0; i < namespace_parts.size(); ++i) {
      output_file_cc << "namespace " << namespace_parts[i] << " {" << std::endl;
    }
    if (!namespace_parts.empty()) {
      output_file_cc << std::endl;
    }
    output_file_cc << "const uint32_t " << FLAGS_output_variable << "["
                   << spirv_data.size() << "] = {" << std::endl;
    size_t i;
    for (i = 0; i + 6 < spirv_data.size(); i += 6) {
      // Write rows of size dwords.
      char line[80 + 1];
      std::snprintf(line, count_of(line),
                    "    0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X,\n",
                    spirv_data[i + 0], spirv_data[i + 1], spirv_data[i + 2],
                    spirv_data[i + 3], spirv_data[i + 4], spirv_data[i + 5]);
      output_file_cc << line;
    }
    if (i < spirv_data.size()) {
      // Write remainder on last row.
      output_file_cc << "    ";
      char dword[16];
      for (; i < spirv_data.size() - 1; ++i) {
        std::snprintf(dword, count_of(dword), "0x%08X, ", spirv_data[i]);
        output_file_cc << dword;
      }
      for (; i < spirv_data.size(); ++i) {
        std::snprintf(dword, count_of(dword), "0x%08X,", spirv_data[i]);
        output_file_cc << dword;
      }
      output_file_cc << std::endl;
    }
    output_file_cc << "};" << std::endl;
    if (!namespace_parts.empty()) {
      output_file_cc << std::endl;
    }
    for (int i = namespace_parts.size() - 1; i >= 0; --i) {
      output_file_cc << "}  // namespace " << namespace_parts[i] << std::endl;
    }
    output_file_cc.close();
  }

  return 0;
}

}  // namespace spirv
}  // namespace gfx
}  // namespace xrtl

DECLARE_CLI_ENTRY_POINT(xrtl::gfx::spirv::ShaderToolMain);
