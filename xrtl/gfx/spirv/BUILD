# Description:
#  SPIR-V tools.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "shader_compiler",
    srcs = ["shader_compiler.cc"],
    hdrs = ["shader_compiler.h"],
    deps = [
        "//xrtl/base:debugging",
        "//xrtl/base:string_view",
        "//xrtl/base:tracing",
        "@com_github_khronosgroup_glslang//:SPIRV",
        "@com_github_khronosgroup_glslang//:glslang",
    ],
)

cc_test(
    name = "shader_compiler_test",
    size = "small",
    srcs = ["shader_compiler_test.cc"],
    deps = [
        ":shader_compiler",
        "//xrtl/testing:gtest_main",
    ],
)

cc_binary(
    name = "shader_tool",
    srcs = ["shader_tool_cli.cc"],
    deps = [
        ":shader_compiler",
        ":spirv_optimizer",
        "//xrtl/base:cli_main",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
        "@spirv_tools//:spirv_tools",
    ],
)

cc_library(
    name = "spirv_optimizer",
    srcs = ["spirv_optimizer.cc"],
    hdrs = ["spirv_optimizer.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:tracing",
        "@spirv_tools//:spirv_tools_opt",
    ],
)

cc_test(
    name = "spirv_optimizer_test",
    size = "small",
    srcs = ["spirv_optimizer_test.cc"],
    deps = [
        ":spirv_optimizer",
        "//xrtl/testing:gtest_main",
    ],
)
