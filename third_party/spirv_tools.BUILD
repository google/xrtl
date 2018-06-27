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
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = [
        "core.insts-1.0.inc",
        "operand.kinds-1.0.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.0) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --core-insts-output=$(location core.insts-1.0.inc) --operand-kinds-output=$(location operand.kinds-1.0.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_core_1.1",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.1",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = [
        "core.insts-1.1.inc",
        "operand.kinds-1.1.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.1) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --core-insts-output=$(location core.insts-1.1.inc) --operand-kinds-output=$(location operand.kinds-1.1.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_core_1.2",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.2",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = [
        "core.insts-1.2.inc",
        "operand.kinds-1.2.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_1.2) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --core-insts-output=$(location core.insts-1.2.inc) --operand-kinds-output=$(location operand.kinds-1.2.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_core_unified1",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = [
        "core.insts-unified1.inc",
        "operand.kinds-unified1.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --core-insts-output=$(location core.insts-unified1.inc) --operand-kinds-output=$(location operand.kinds-unified1.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_enum_string_mapping_unified1",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = [
        "extension_enum.inc",
        "enum_string_mapping.inc",
    ],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --extension-enum-output=$(location extension_enum.inc) --enum-string-mapping-output=$(location enum_string_mapping.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_opencl_unified1",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1",
        "@com_github_khronosgroup_spirv_headers//:spirv_opencl_grammar_unified1",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = ["opencl.std.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --extinst-opencl-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_opencl_grammar_unified1) --opencl-insts-output=$(location opencl.std.insts.inc)",
    tools = [":generate_grammar_tables"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_grammar_tables_glsl_unified1",
    srcs = [
        "@com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1",
        "@com_github_khronosgroup_spirv_headers//:spirv_glsl_grammar_unified1",
        "source/extinst.debuginfo.grammar.json",
    ],
    outs = ["glsl.std.450.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --spirv-core-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_core_grammar_unified1) --extinst-debuginfo-grammar=$(location source/extinst.debuginfo.grammar.json) --extinst-glsl-grammar=$(location @com_github_khronosgroup_spirv_headers//:spirv_glsl_grammar_unified1) --glsl-insts-output=$(location glsl.std.450.insts.inc)",
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

genrule(
    name = "run_generate_grammar_tables_debuginfo",
    srcs = ["source/extinst.debuginfo.grammar.json"],
    outs = ["debuginfo.insts.inc"],
    cmd = "$(location :generate_grammar_tables) --extinst-vendor-grammar=$(location source/extinst.debuginfo.grammar.json) --vendor-insts-output=$(location debuginfo.insts.inc)",
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

py_binary(
    name = "generate_language_headers",
    srcs = ["utils/generate_language_headers.py"],
    visibility = ["//visibility:private"],
)

genrule(
    name = "run_generate_lang_headers_DebugInfo",
    srcs = ["source/extinst.debuginfo.grammar.json"],
    outs = ["DebugInfo.h"],
    cmd = "$(location :generate_language_headers) --extinst-name=DebugInfo --extinst-grammar=$(location source/extinst.debuginfo.grammar.json) --extinst-output-base=$(@D)/DebugInfo",
    tools = [":generate_language_headers"],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "generated_headers",
    srcs = [
        ":run_generate_grammar_tables_core_1.0",
        ":run_generate_grammar_tables_core_1.1",
        ":run_generate_grammar_tables_core_1.2",
        ":run_generate_grammar_tables_core_unified1",
        ":run_generate_grammar_tables_debuginfo",
        ":run_generate_grammar_tables_enum_string_mapping_unified1",
        ":run_generate_grammar_tables_glsl_unified1",
        ":run_generate_grammar_tables_opencl_unified1",
        ":run_generate_grammar_tables_spv_amd_gcn_shader",
        ":run_generate_grammar_tables_spv_amd_shader_ballot",
        ":run_generate_grammar_tables_spv_amd_shader_explicit_vertex_parameter",
        ":run_generate_grammar_tables_spv_amd_trinary_minmax",
        ":run_generate_lang_headers_DebugInfo",
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
        "source/comp/markv.h",
        "source/comp/markv_codec.cpp",
        "source/comp/markv_model.h",
        "source/diagnostic.cpp",
        "source/diagnostic.h",
        "source/disassemble.cpp",
        "source/disassemble.h",
        "source/enum_set.h",
        "source/enum_string_mapping.cpp",
        "source/enum_string_mapping.h",
        "source/ext_inst.cpp",
        "source/ext_inst.h",
        "source/extensions.cpp",
        "source/extensions.h",
        "source/id_descriptor.cpp",
        "source/id_descriptor.h",
        "source/instruction.h",
        "source/latest_version_glsl_std_450_header.h",
        "source/latest_version_opencl_std_header.h",
        "source/latest_version_spirv_header.h",
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
        "source/util/bit_vector.cpp",
        "source/util/bit_vector.h",
        "source/util/bitutils.h",
        "source/util/hex_float.h",
        "source/util/huffman_codec.h",
        "source/util/ilist.h",
        "source/util/ilist_node.h",
        "source/util/move_to_front.h",
        "source/util/parse_number.cpp",
        "source/util/parse_number.h",
        "source/util/small_vector.h",
        "source/util/string_utils.cpp",
        "source/util/string_utils.h",
        "source/util/timer.h",
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
        "source/validate_adjacency.cpp",
        "source/validate_arithmetics.cpp",
        "source/validate_atomics.cpp",
        "source/validate_barriers.cpp",
        "source/validate_bitwise.cpp",
        "source/validate_builtins.cpp",
        "source/validate_capability.cpp",
        "source/validate_cfg.cpp",
        "source/validate_composites.cpp",
        "source/validate_conversion.cpp",
        "source/validate_datarules.cpp",
        "source/validate_decorations.cpp",
        "source/validate_derivatives.cpp",
        "source/validate_ext_inst.cpp",
        "source/validate_id.cpp",
        "source/validate_image.cpp",
        "source/validate_instruction.cpp",
        "source/validate_interfaces.cpp",
        "source/validate_layout.cpp",
        "source/validate_literals.cpp",
        "source/validate_logicals.cpp",
        "source/validate_non_uniform.cpp",
        "source/validate_primitives.cpp",
        "source/validate_type_unique.cpp",
    ] + select({
        "@//xrtl/tools/target_platform:windows": [],
        "//conditions:default": [
            # Timer classes use POSIX-only sys/ includes. The timer is only
            # enabled if SPIRV_TIMER_ENABLED is defined though.
            # SPIRV-Tools conditionally includes this source file via CMake.
            "source/util/timer.cpp",
        ],
    }),
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
        "source/link/linker.cpp",
        "source/opt/aggressive_dead_code_elim_pass.cpp",
        "source/opt/aggressive_dead_code_elim_pass.h",
        "source/opt/basic_block.cpp",
        "source/opt/basic_block.h",
        "source/opt/block_merge_pass.cpp",
        "source/opt/block_merge_pass.h",
        "source/opt/build_module.cpp",
        "source/opt/build_module.h",
        "source/opt/ccp_pass.cpp",
        "source/opt/ccp_pass.h",
        "source/opt/cfg.cpp",
        "source/opt/cfg.h",
        "source/opt/cfg_cleanup_pass.cpp",
        "source/opt/cfg_cleanup_pass.h",
        "source/opt/common_uniform_elim_pass.cpp",
        "source/opt/common_uniform_elim_pass.h",
        "source/opt/compact_ids_pass.cpp",
        "source/opt/compact_ids_pass.h",
        "source/opt/composite.cpp",
        "source/opt/composite.h",
        "source/opt/const_folding_rules.cpp",
        "source/opt/const_folding_rules.h",
        "source/opt/constants.cpp",
        "source/opt/constants.h",
        "source/opt/copy_prop_arrays.cpp",
        "source/opt/copy_prop_arrays.h",
        "source/opt/dead_branch_elim_pass.cpp",
        "source/opt/dead_branch_elim_pass.h",
        "source/opt/dead_insert_elim_pass.cpp",
        "source/opt/dead_insert_elim_pass.h",
        "source/opt/dead_variable_elimination.cpp",
        "source/opt/dead_variable_elimination.h",
        "source/opt/decoration_manager.cpp",
        "source/opt/decoration_manager.h",
        "source/opt/def_use_manager.cpp",
        "source/opt/def_use_manager.h",
        "source/opt/dominator_analysis.cpp",
        "source/opt/dominator_analysis.h",
        "source/opt/dominator_tree.cpp",
        "source/opt/dominator_tree.h",
        "source/opt/eliminate_dead_constant_pass.cpp",
        "source/opt/eliminate_dead_constant_pass.h",
        "source/opt/eliminate_dead_functions_pass.cpp",
        "source/opt/eliminate_dead_functions_pass.h",
        "source/opt/feature_manager.cpp",
        "source/opt/feature_manager.h",
        "source/opt/flatten_decoration_pass.cpp",
        "source/opt/flatten_decoration_pass.h",
        "source/opt/fold.cpp",
        "source/opt/fold.h",
        "source/opt/fold_spec_constant_op_and_composite_pass.cpp",
        "source/opt/fold_spec_constant_op_and_composite_pass.h",
        "source/opt/folding_rules.cpp",
        "source/opt/folding_rules.h",
        "source/opt/freeze_spec_constant_value_pass.cpp",
        "source/opt/freeze_spec_constant_value_pass.h",
        "source/opt/function.cpp",
        "source/opt/function.h",
        "source/opt/if_conversion.cpp",
        "source/opt/if_conversion.h",
        "source/opt/inline_exhaustive_pass.cpp",
        "source/opt/inline_exhaustive_pass.h",
        "source/opt/inline_opaque_pass.cpp",
        "source/opt/inline_opaque_pass.h",
        "source/opt/inline_pass.cpp",
        "source/opt/inline_pass.h",
        "source/opt/instruction.cpp",
        "source/opt/instruction.h",
        "source/opt/instruction_list.cpp",
        "source/opt/instruction_list.h",
        "source/opt/ir_builder.h",
        "source/opt/ir_context.cpp",
        "source/opt/ir_context.h",
        "source/opt/ir_loader.cpp",
        "source/opt/ir_loader.h",
        "source/opt/iterator.h",
        "source/opt/licm_pass.cpp",
        "source/opt/licm_pass.h",
        "source/opt/local_access_chain_convert_pass.cpp",
        "source/opt/local_access_chain_convert_pass.h",
        "source/opt/local_redundancy_elimination.cpp",
        "source/opt/local_redundancy_elimination.h",
        "source/opt/local_single_block_elim_pass.cpp",
        "source/opt/local_single_block_elim_pass.h",
        "source/opt/local_single_store_elim_pass.cpp",
        "source/opt/local_single_store_elim_pass.h",
        "source/opt/local_ssa_elim_pass.cpp",
        "source/opt/local_ssa_elim_pass.h",
        "source/opt/log.h",
        "source/opt/loop_dependence.cpp",
        "source/opt/loop_dependence.h",
        "source/opt/loop_dependence_helpers.cpp",
        "source/opt/loop_descriptor.cpp",
        "source/opt/loop_descriptor.h",
        "source/opt/loop_fission.cpp",
        "source/opt/loop_fission.h",
        "source/opt/loop_fusion.cpp",
        "source/opt/loop_fusion.h",
        "source/opt/loop_fusion_pass.cpp",
        "source/opt/loop_fusion_pass.h",
        "source/opt/loop_peeling.cpp",
        "source/opt/loop_peeling.h",
        "source/opt/loop_unroller.cpp",
        "source/opt/loop_unroller.h",
        "source/opt/loop_unswitch_pass.cpp",
        "source/opt/loop_unswitch_pass.h",
        "source/opt/loop_utils.cpp",
        "source/opt/loop_utils.h",
        "source/opt/make_unique.h",
        "source/opt/mem_pass.cpp",
        "source/opt/mem_pass.h",
        "source/opt/merge_return_pass.cpp",
        "source/opt/merge_return_pass.h",
        "source/opt/module.cpp",
        "source/opt/module.h",
        "source/opt/null_pass.h",
        "source/opt/optimizer.cpp",
        "source/opt/pass.cpp",
        "source/opt/pass.h",
        "source/opt/pass_manager.cpp",
        "source/opt/pass_manager.h",
        "source/opt/passes.h",
        "source/opt/private_to_local_pass.cpp",
        "source/opt/private_to_local_pass.h",
        "source/opt/propagator.cpp",
        "source/opt/propagator.h",
        "source/opt/reduce_load_size.cpp",
        "source/opt/reduce_load_size.h",
        "source/opt/redundancy_elimination.cpp",
        "source/opt/redundancy_elimination.h",
        "source/opt/reflect.h",
        "source/opt/register_pressure.cpp",
        "source/opt/register_pressure.h",
        "source/opt/remove_duplicates_pass.cpp",
        "source/opt/remove_duplicates_pass.h",
        "source/opt/replace_invalid_opc.cpp",
        "source/opt/replace_invalid_opc.h",
        "source/opt/scalar_analysis.cpp",
        "source/opt/scalar_analysis.h",
        "source/opt/scalar_analysis_nodes.h",
        "source/opt/scalar_analysis_simplification.cpp",
        "source/opt/scalar_replacement_pass.cpp",
        "source/opt/scalar_replacement_pass.h",
        "source/opt/set_spec_constant_default_value_pass.cpp",
        "source/opt/set_spec_constant_default_value_pass.h",
        "source/opt/simplification_pass.cpp",
        "source/opt/simplification_pass.h",
        "source/opt/ssa_rewrite_pass.cpp",
        "source/opt/ssa_rewrite_pass.h",
        "source/opt/strength_reduction_pass.cpp",
        "source/opt/strength_reduction_pass.h",
        "source/opt/strip_debug_info_pass.cpp",
        "source/opt/strip_debug_info_pass.h",
        "source/opt/strip_reflect_info_pass.cpp",
        "source/opt/strip_reflect_info_pass.h",
        "source/opt/tree_iterator.h",
        "source/opt/type_manager.cpp",
        "source/opt/type_manager.h",
        "source/opt/types.cpp",
        "source/opt/types.h",
        "source/opt/unify_const_pass.cpp",
        "source/opt/unify_const_pass.h",
        "source/opt/value_number_table.cpp",
        "source/opt/value_number_table.h",
        "source/opt/vector_dce.cpp",
        "source/opt/vector_dce.h",
        "source/opt/workaround1209.cpp",
        "source/opt/workaround1209.h",
    ],
    hdrs = [
        "include/spirv-tools/linker.hpp",
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
