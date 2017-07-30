# Description:
#  Library for performing reflection on SPIR-V and disassembling SPIR-V
#  bytecode.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

COMMON_COPTS = [
    "-DDISABLE_GOOGLE_STRING",
] + select({
    "@//xrtl/tools/target_platform:windows": [],
    "//conditions:default": [
        "-fexceptions",
    ],
})

cc_library(
    name = "spirv_cross_lib",
    srcs = [
        "GLSL.std.450.h",
        "spirv_cfg.cpp",
        "spirv_cpp.cpp",
        "spirv_cross.cpp",
        "spirv_glsl.cpp",
        "spirv_hlsl.cpp",
        "spirv_msl.cpp",
    ],
    hdrs = [
        "spirv.hpp",
        "spirv_cfg.hpp",
        "spirv_common.hpp",
        "spirv_cpp.hpp",
        "spirv_cross.hpp",
        "spirv_glsl.hpp",
        "spirv_hlsl.hpp",
        "spirv_msl.hpp",
    ],
    copts = COMMON_COPTS,
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "spirv_cross",
    srcs = ["main.cpp"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [":spirv_cross_lib"],
)
