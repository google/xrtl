# Description:
#  Graphics abstraction layer.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "pixel_format",
    srcs = ["pixel_format.cc"],
    hdrs = ["pixel_format.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:macros",
    ],
)

cc_test(
    name = "pixel_format_test",
    srcs = ["pixel_format_test.cc"],
    deps = [
        ":pixel_format",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
)
