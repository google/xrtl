# Description:
#  Diffing utilities for various content types.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "portable_cc_test")

cc_library(
    name = "data_differ",
    testonly = 1,
    srcs = ["data_differ.cc"],
    hdrs = ["data_differ.h"],
    deps = [
        "//xrtl/base:logging",
    ],
)

portable_cc_test(
    name = "data_differ_test",
    size = "small",
    srcs = ["data_differ_test.cc"],
    deps = [
        ":data_differ",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "diff_provider",
    testonly = 1,
    srcs = ["diff_provider.cc"],
    hdrs = ["diff_provider.h"],
    deps = [
        ":data_differ",
        ":image_differ",
        ":text_differ",
        "//xrtl/base:logging",
        "//xrtl/testing:file_util",
        "//xrtl/testing:image_buffer",
        "@com_google_absl//absl/strings",
    ],
)

portable_cc_test(
    name = "diff_provider_test",
    size = "small",
    srcs = ["diff_provider_test.cc"],
    data = [
        "//xrtl/testing/diffing/testdata:test_files",
        "//xrtl/testing/diffing/testdata/goldens:test_files",
    ],
    deps = [
        ":diff_provider",
        "//xrtl/testing:file_util",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "html_diff_provider",
    testonly = 1,
    srcs = ["html_diff_provider.cc"],
    hdrs = ["html_diff_provider.h"],
    deps = [
        ":diff_provider",
        "//xrtl/testing:file_util",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "html_diff_provider_create",
    testonly = 1,
    srcs = ["html_diff_provider_create.cc"],
    deps = [
        ":html_diff_provider",
        "//xrtl/base:macros",
    ],
)

cc_library(
    name = "image_differ",
    testonly = 1,
    srcs = ["image_differ.cc"],
    hdrs = ["image_differ.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/testing:image_buffer",
    ],
)

portable_cc_test(
    name = "image_differ_test",
    size = "small",
    srcs = ["image_differ_test.cc"],
    data = [
        "//xrtl/testing/diffing/testdata:test_files",
    ],
    deps = [
        ":image_differ",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "text_differ",
    testonly = 1,
    srcs = ["text_differ.cc"],
    hdrs = ["text_differ.h"],
    deps = [
        "//xrtl/base:logging",
        "@com_google_absl//absl/strings",
    ],
)

portable_cc_test(
    name = "text_differ_test",
    size = "small",
    srcs = ["text_differ_test.cc"],
    deps = [
        ":text_differ",
        "//xrtl/testing:gtest_main",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
