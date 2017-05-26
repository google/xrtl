# Description:
#  SPIR-V tools.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "shader_compiler",
    srcs = ["shader_compiler.cc"],
    hdrs = ["shader_compiler.h"],
    deps = [
        "//external:glslang",
        "//external:glslang_spirv",
        "//xrtl/base:debugging",
        "//xrtl/base:string_view",
        "//xrtl/base:tracing",
    ],
)

cc_test(
    name = "shader_compiler_test",
    srcs = ["shader_compiler_test.cc"],
    deps = [
        ":shader_compiler",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "spirv_optimizer",
    srcs = ["spirv_optimizer.cc"],
    hdrs = ["spirv_optimizer.h"],
    deps = [
        "//external:spirv_tools_opt",
        "//xrtl/base:logging",
        "//xrtl/base:tracing",
    ],
)

cc_test(
    name = "spirv_optimizer_test",
    srcs = ["spirv_optimizer_test.cc"],
    deps = [
        ":spirv_optimizer",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
)
