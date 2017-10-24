# Description:
#  Emscripten port variants of //xrtl/base/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = [
    "//xrtl/tools/target_platform:emscripten",
])

cc_library(
    name = "emscripten_logging_sink",
    srcs = ["emscripten_logging_sink.cc"],
    deps = [
        "//xrtl/base:logging_hdrs",
        "//xrtl/port/common/base:logging_macros_hdrs",
    ],
)
