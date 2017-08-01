# Description:
#  The SPIR-V Tools project provides an API and commands for processing SPIR-V
#  modules.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

COMMON_COPTS = [
    "-DDISABLE_GOOGLE_GLOBAL_USING_DECLARATIONS",
    "-DDISABLE_GOOGLE_STRING",
    "-DSPIRV_LINUX",
    "-DSPIRV_COLOR_TERMINAL",
    "-Iexternal/spirv_tools/source/",
]

py_binary(
    name = "generate_grammar_tables",
    srcs = ["utils/generate_grammar_tables.py"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_core_1.0",
    srcs = ["@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0"],
    outs = [
        "core.insts-1.0.inc",
        "operand.kinds-1.0.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0) --core-insts-output=$(location core.insts-1.0.inc) --operand-kinds-output=$(location operand.kinds-1.0.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_core_1.1",
    srcs = ["@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.1"],
    outs = [
        "core.insts-1.1.inc",
        "operand.kinds-1.1.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.1) --core-insts-output=$(location core.insts-1.1.inc) --operand-kinds-output=$(location operand.kinds-1.1.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_core_1.2",
    srcs = ["@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.2"],
    outs = [
        "core.insts-1.2.inc",
        "operand.kinds-1.2.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.2) --core-insts-output=$(location core.insts-1.2.inc) --operand-kinds-output=$(location operand.kinds-1.2.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_enum_string_mapping_1.2",
    srcs = ["@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.2"],
    outs = [
        "extension_enum.inc",
        "enum_string_mapping.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.2) --extension-enum-output=$(location extension_enum.inc) --enum-string-mapping-output=$(location enum_string_mapping.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_opencl_1.0",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0",
        "@com_github_khronosgroup_spirv_headers//:spirv_opencl_grammar_1.0",
    ],
    outs = ["opencl.std.insts-1.0.inc"],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0) --extinst-opencl-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_opencl_grammar_1.0) --opencl-insts-output=$(location opencl.std.insts-1.0.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_glsl_1.0",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0",
        "@com_github_khronosgroup_spirv_headers//:spirv_glsl_grammar_1.0",
    ],
    outs = ["glsl.std.450.insts-1.0.inc"],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0) --extinst-glsl-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_glsl_grammar_1.0) --glsl-insts-output=$(location glsl.std.450.insts-1.0.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_spv_amd_gcn_shader",
    srcs = ["source/extinst.spv-amd-gcn-shader.grammar.json"],
    outs = ["spv-amd-gcn-shader.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --extinst-vendor-grammar=$(location source/extinst.spv-amd-gcn-shader.grammar.json) --vendor-insts-output=$(location spv-amd-gcn-shader.insts.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_spv_amd_shader_ballot",
    srcs = ["source/extinst.spv-amd-shader-ballot.grammar.json"],
    outs = ["spv-amd-shader-ballot.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --extinst-vendor-grammar=$(location source/extinst.spv-amd-shader-ballot.grammar.json) --vendor-insts-output=$(location spv-amd-shader-ballot.insts.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_spv_amd_shader_explicit_vertex_parameter",
    srcs = ["source/extinst.spv-amd-shader-explicit-vertex-parameter.grammar.json"],
    outs = ["spv-amd-shader-explicit-vertex-parameter.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --extinst-vendor-grammar=$(location source/extinst.spv-amd-shader-explicit-vertex-parameter.grammar.json) --vendor-insts-output=$(location spv-amd-shader-explicit-vertex-parameter.insts.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_spv_amd_trinary_minmax",
    srcs = ["source/extinst.spv-amd-shader-trinary-minmax.grammar.json"],
    outs = ["spv-amd-shader-trinary-minmax.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --extinst-vendor-grammar=$(location source/extinst.spv-amd-shader-trinary-minmax.grammar.json) --vendor-insts-output=$(location spv-amd-shader-trinary-minmax.insts.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

py_binary(
    name = "generate_registry_tables",
    srcs = ["utils/generate_registry_tables.py"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_registry_tables",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_xml_registry",
    ],
    outs = ["generators.inc"],
    cmd = "$(location :generate_registry_tables) --xml=$(location @com_github_khronosgroup_spirv_headers//:spirv_xml_registry) --generator-output=$(location generators.inc)",
    tools = [":generate_registry_tables"],
    visibility = ["//visibility:private"],
)

py_binary(
    name = "update_build_version",
    srcs = ["utils/update_build_version.py"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_update_build_version",
    srcs = [
        "CHANGES",
    ],
    outs = ["build-version.inc"],
    cmd = "SOURCE_DATE_EPOCH=0 $(location :update_build_version) $$(dirname $(location CHANGES)) $(location build-version.inc)",
    tools = [":update_build_version"],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "generated_headers",
    srcs = [
        ":run_generate_grammar_tables_core_1.0",
        ":run_generate_grammar_tables_core_1.1",
        ":run_generate_grammar_tables_core_1.2",
        ":run_generate_grammar_tables_enum_string_mapping_1.2",
        ":run_generate_grammar_tables_glsl_1.0",
        ":run_generate_grammar_tables_opencl_1.0",
        ":run_generate_grammar_tables_spv_amd_gcn_shader",
        ":run_generate_grammar_tables_spv_amd_shader_ballot",
        ":run_generate_grammar_tables_spv_amd_shader_explicit_vertex_parameter",
        ":run_generate_grammar_tables_spv_amd_trinary_minmax",
        ":run_generate_registry_tables",
        ":run_update_build_version",
    ],
    includes = [
        ".",
        "external/spirv_tools/source/",
    ],
    linkstatic = 1,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "spirv_tools",
    srcs = [
        "source/assembly_grammar.cpp",
        "source/assembly_grammar.h",
        "source/binary.cpp",
        "source/binary.h",
        "source/cfa.h",
        "source/diagnostic.cpp",
        "source/diagnostic.h",
        "source/disassemble.cpp",
        "source/enum_set.h",
        "source/enum_string_mapping.cpp",
        "source/enum_string_mapping.h",
        "source/ext_inst.cpp",
        "source/ext_inst.h",
        "source/extensions.cpp",
        "source/extensions.h",
        "source/instruction.h",
        "source/libspirv.cpp",
        "source/macro.h",
        "source/message.cpp",
        "source/message.h",
        "source/name_mapper.cpp",
        "source/name_mapper.h",
        "source/opcode.cpp",
        "source/opcode.h",
        "source/operand.cpp",
        "source/operand.h",
        "source/parsed_operand.cpp",
        "source/parsed_operand.h",
        "source/print.cpp",
        "source/print.h",
        "source/software_version.cpp",
        "source/spirv_constant.h",
        "source/spirv_definition.h",
        "source/spirv_endian.cpp",
        "source/spirv_endian.h",
        "source/spirv_stats.cpp",
        "source/spirv_stats.h",
        "source/spirv_target_env.cpp",
        "source/spirv_target_env.h",
        "source/spirv_validator_options.cpp",
        "source/spirv_validator_options.h",
        "source/table.cpp",
        "source/table.h",
        "source/text.cpp",
        "source/text.h",
        "source/text_handler.cpp",
        "source/text_handler.h",
        "source/util/bit_stream.cpp",
        "source/util/bit_stream.h",
        "source/util/bitutils.h",
        "source/util/hex_float.h",
        "source/util/huffman_codec.h",
        "source/util/move_to_front.h",
        "source/util/parse_number.cpp",
        "source/util/parse_number.h",
        "source/util/string_utils.cpp",
        "source/util/string_utils.h",
        "source/val/basic_block.cpp",
        "source/val/basic_block.h",
        "source/val/construct.cpp",
        "source/val/construct.h",
        "source/val/decoration.h",
        "source/val/function.cpp",
        "source/val/function.h",
        "source/val/instruction.cpp",
        "source/val/instruction.h",
        "source/val/validation_state.cpp",
        "source/val/validation_state.h",
        "source/validate.cpp",
        "source/validate.h",
        "source/validate_capability.cpp",
        "source/validate_cfg.cpp",
        "source/validate_datarules.cpp",
        "source/validate_decorations.cpp",
        "source/validate_id.cpp",
        "source/validate_instruction.cpp",
        "source/validate_layout.cpp",
        "source/validate_type_unique.cpp",
    ],
    hdrs = [
        "include/spirv-tools/libspirv.h",
        "include/spirv-tools/libspirv.hpp",
    ],
    copts = COMMON_COPTS,
    includes = ["include"],
    deps = [
        ":generated_headers",
        "@com_github_khronosgroup_spirv_headers//:spirv_c_headers",
        "@com_github_khronosgroup_spirv_headers//:spirv_common_headers",
    ],
)

cc_library(
    name = "spirv_tools_opt",
    srcs = [
        "source/opt/aggressive_dead_code_elim_pass.cpp",
        "source/opt/aggressive_dead_code_elim_pass.h",
        "source/opt/basic_block.cpp",
        "source/opt/basic_block.h",
        "source/opt/block_merge_pass.cpp",
        "source/opt/block_merge_pass.h",
        "source/opt/build_module.cpp",
        "source/opt/build_module.h",
        "source/opt/compact_ids_pass.cpp",
        "source/opt/compact_ids_pass.h",
        "source/opt/constants.h",
        "source/opt/dead_branch_elim_pass.cpp",
        "source/opt/dead_branch_elim_pass.h",
        "source/opt/def_use_manager.cpp",
        "source/opt/def_use_manager.h",
        "source/opt/eliminate_dead_constant_pass.cpp",
        "source/opt/eliminate_dead_constant_pass.h",
        "source/opt/flatten_decoration_pass.cpp",
        "source/opt/flatten_decoration_pass.h",
        "source/opt/fold_spec_constant_op_and_composite_pass.cpp",
        "source/opt/fold_spec_constant_op_and_composite_pass.h",
        "source/opt/freeze_spec_constant_value_pass.cpp",
        "source/opt/freeze_spec_constant_value_pass.h",
        "source/opt/function.cpp",
        "source/opt/function.h",
        "source/opt/inline_pass.cpp",
        "source/opt/inline_pass.h",
        "source/opt/insert_extract_elim.cpp",
        "source/opt/insert_extract_elim.h",
        "source/opt/instruction.cpp",
        "source/opt/instruction.h",
        "source/opt/ir_loader.cpp",
        "source/opt/ir_loader.h",
        "source/opt/iterator.h",
        "source/opt/local_access_chain_convert_pass.cpp",
        "source/opt/local_access_chain_convert_pass.h",
        "source/opt/local_single_block_elim_pass.cpp",
        "source/opt/local_single_block_elim_pass.h",
        "source/opt/local_single_store_elim_pass.cpp",
        "source/opt/local_single_store_elim_pass.h",
        "source/opt/local_ssa_elim_pass.cpp",
        "source/opt/local_ssa_elim_pass.h",
        "source/opt/log.h",
        "source/opt/make_unique.h",
        "source/opt/module.cpp",
        "source/opt/module.h",
        "source/opt/null_pass.h",
        "source/opt/optimizer.cpp",
        "source/opt/pass.h",
        "source/opt/pass_manager.cpp",
        "source/opt/pass_manager.h",
        "source/opt/passes.h",
        "source/opt/reflect.h",
        "source/opt/set_spec_constant_default_value_pass.cpp",
        "source/opt/set_spec_constant_default_value_pass.h",
        "source/opt/strip_debug_info_pass.cpp",
        "source/opt/strip_debug_info_pass.h",
        "source/opt/type_manager.cpp",
        "source/opt/type_manager.h",
        "source/opt/types.cpp",
        "source/opt/types.h",
        "source/opt/unify_const_pass.cpp",
        "source/opt/unify_const_pass.h",
    ],
    hdrs = [
        "include/spirv-tools/optimizer.hpp",
    ],
    copts = COMMON_COPTS,
    includes = ["include"],
    deps = [
        ":spirv_tools",
        "@com_github_khronosgroup_spirv_headers//:spirv_common_headers",
    ],
)

cc_binary(
    name = "spirv-as",
    srcs = [
        "tools/as/as.cpp",
        "tools/io.h",
    ],
    copts = COMMON_COPTS + [
        "-I.",
    ],
    deps = [
        ":spirv_tools",
    ],
)

cc_binary(
    name = "spirv-dis",
    srcs = [
        "tools/dis/dis.cpp",
        "tools/io.h",
    ],
    copts = COMMON_COPTS + [
        "-I.",
    ],
    deps = [
        ":spirv_tools",
    ],
)

cc_binary(
    name = "spirv-val",
    srcs = [
        "tools/io.h",
        "tools/val/val.cpp",
    ],
    copts = COMMON_COPTS + [
        "-I.",
    ],
    deps = [
        ":spirv_tools",
    ],
)

cc_binary(
    name = "spirv-opt",
    srcs = [
        "tools/io.h",
        "tools/opt/opt.cpp",
    ],
    copts = COMMON_COPTS + [
        "-I.",
    ],
    deps = [
        ":spirv_tools",
        ":spirv_tools_opt",
    ],
)

cc_binary(
    name = "spirv-cfg",
    srcs = [
        "tools/cfg/bin_to_dot.cpp",
        "tools/cfg/bin_to_dot.h",
        "tools/cfg/cfg.cpp",
        "tools/io.h",
    ],
    copts = COMMON_COPTS + [
        "-I.",
    ],
    deps = [
        ":spirv_tools",
    ],
)
