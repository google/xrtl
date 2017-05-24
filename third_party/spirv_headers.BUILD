# Description:
#  This repository contains machine-readable files from the SPIR-V Registry.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Khronos MIT

exports_files(["LICENSE"])

filegroup(
    name = "spirv_core_grammar_1.0",
    srcs = ["include/spirv/1.0/spirv.core.grammar.json"],
)

filegroup(
    name = "spirv_glsl_grammar_1.0",
    srcs = ["include/spirv/1.0/extinst.glsl.std.450.grammar.json"],
)

filegroup(
    name = "spirv_opencl_grammar_1.0",
    srcs = ["include/spirv/1.0/extinst.opencl.std.100.grammar.json"],
)

filegroup(
    name = "spirv_core_grammar_1.1",
    srcs = ["include/spirv/1.1/spirv.core.grammar.json"],
)

filegroup(
    name = "spirv_core_grammar_1.2",
    srcs = ["include/spirv/1.2/spirv.core.grammar.json"],
)

filegroup(
    name = "spirv_glsl_grammar_1.1",
    srcs = ["include/spirv/1.1/extinst.glsl.std.450.grammar.json"],
)

filegroup(
    name = "spirv_glsl_grammar_1.2",
    srcs = ["include/spirv/1.2/extinst.glsl.std.450.grammar.json"],
)

filegroup(
    name = "spirv_opencl_grammar_1.1",
    srcs = ["include/spirv/1.1/extinst.opencl.std.100.grammar.json"],
)

filegroup(
    name = "spirv_opencl_grammar_1.2",
    srcs = ["include/spirv/1.2/extinst.opencl.std.100.grammar.json"],
)

filegroup(
    name = "spirv_xml_registry",
    srcs = ["include/spirv/spir-v.xml"],
)

cc_library(
    name = "spirv_common_headers",
    hdrs = [
        "include/spirv/1.0/GLSL.std.450.h",
        "include/spirv/1.0/OpenCL.std.h",
        "include/spirv/1.1/GLSL.std.450.h",
        "include/spirv/1.1/OpenCL.std.h",
        "include/spirv/1.2/GLSL.std.450.h",
        "include/spirv/1.2/OpenCL.std.h",
    ],
)

cc_library(
    name = "spirv_c_headers",
    hdrs = [
        "include/spirv/1.0/spirv.h",
        "include/spirv/1.1/spirv.h",
        "include/spirv/1.2/spirv.h",
    ],
    deps = [
        ":spirv_common_headers",
    ],
    includes = ["include/"],
)

cc_library(
    name = "spirv_cpp_headers",
    hdrs = [
        "include/spirv/1.0/spirv.hpp",
        "include/spirv/1.1/spirv.hpp",
        "include/spirv/1.2/spirv.hpp",
    ],
    deps = [
        ":spirv_common_headers",
    ],
    includes = ["include/"],
)

cc_library(
    name = "spirv_cpp11_headers",
    hdrs = [
        "include/spirv/1.0/spirv.hpp11",
        "include/spirv/1.1/spirv.hpp11",
        "include/spirv/1.2/spirv.hpp11",
    ],
    deps = [
        ":spirv_common_headers",
    ],
    includes = ["include/"],
)
