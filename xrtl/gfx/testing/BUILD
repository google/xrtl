# Description:
#  Graphics testing utilties.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "portable_cc_test")

cc_library(
    name = "graphics_test",
    testonly = 1,
    srcs = ["graphics_test.cc"],
    hdrs = ["graphics_test.h"],
    deps = [
        "//xrtl/base:tracing",
        "//xrtl/gfx:context",
        "//xrtl/gfx:context_factory",
        "//xrtl/testing:gtest",
        "//xrtl/testing/diffing:diff_provider",
        "@com_google_absl//absl/strings",
    ],
)

portable_cc_test(
    name = "graphics_test_test",
    size = "small",
    srcs = ["graphics_test_test.cc"],
    data = [
        "//xrtl/gfx/testing/testdata/goldens:graphics_test_data",
    ],
    deps = [
        ":graphics_test",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "partial_command_encoder",
    testonly = 1,
    hdrs = ["partial_command_encoder.h"],
    deps = [
        "//xrtl/gfx:command_encoder",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
