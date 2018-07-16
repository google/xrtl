# Description:
#  Testing utilities.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "portable_cc_test")

# demo main() entry point.
# This is linked in to demo cc_binaries.
cc_library(
    name = "demo_main",
    srcs = ["demo_main.cc"],
    hdrs = ["demo_main.h"],
    deps = [
        ":platform_demo_main",
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
    ],
    alwayslink = 1,
)

cc_library(
    name = "demo_main_hdrs",
    hdrs = ["demo_main.h"],
    visibility = ["//xrtl/port:ports_only"],
)

cc_library(
    name = "platform_demo_main",
    visibility = ["//visibility:private"],
    deps = select({
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/testing:win32_demo_main",
        ],
        "//conditions:default": [
            "//xrtl/port/common/testing:posix_demo_main",
        ],
    }),
)

sh_test(
    name = "empty_test",
    srcs = ["empty_test.sh"],
)

cc_library(
    name = "file_util",
    testonly = 1,
    srcs = ["file_util.cc"],
    hdrs = ["file_util.h"],
    deps = [
        "//xrtl/base:env",
        "//xrtl/base:logging",
        "//xrtl/base:tracing",
        "//xrtl/tools/target_platform",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

portable_cc_test(
    name = "file_util_test",
    size = "small",
    srcs = ["file_util_test.cc"],
    data = [
        "//xrtl/testing/testdata:file_util_data",
    ],
    deps = [
        ":file_util",
        "//xrtl/base:env",
        "//xrtl/testing:gtest_main",
    ],
)

# googletest header shim.
# This can be linked in to cc_library rules that require access to the gunit
# headers (such as test utility libraries/etc).
cc_library(
    name = "gtest",
    testonly = 1,
    hdrs = ["gtest.h"],
    deps = [
        "//third_party/gtest_xrtl",
    ],
)

# googletest main() entry point.
# This is linked in to cc_tests.
cc_library(
    name = "gtest_main",
    testonly = 1,
    hdrs = ["gtest.h"],
    deps = [
        ":gtest",
        "//third_party/gtest_xrtl",
    ] + select({
        "//xrtl/tools/target_config:google_internal": [
            "//xrtl/port/google/testing:google_test_main",
        ],
        "//conditions:default": [
            ":platform_gtest_main",
        ],
    }),
)

cc_library(
    name = "platform_gtest_main",
    testonly = 1,
    visibility = ["//visibility:private"],
    deps = [
        "//xrtl/testing/diffing:html_diff_provider_create",
    ] + select({
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:windows": [
            # TODO(benvanik): win32-specific test with better console output.
            "//xrtl/port/common/testing:posix_test_main",
        ],
        "//conditions:default": [
            "//xrtl/port/common/testing:posix_test_main",
        ],
    }),
)

cc_library(
    name = "image_buffer",
    testonly = 1,
    srcs = ["image_buffer.cc"],
    hdrs = ["image_buffer.h"],
    deps = [
        ":file_util",
        "//xrtl/base:logging",
        "//xrtl/base:tracing",
        "@com_github_nothings_stb//:stb_image",
        "@com_github_nothings_stb//:stb_image_write",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:optional",
    ],
)

portable_cc_test(
    name = "image_buffer_test",
    size = "small",
    srcs = ["image_buffer_test.cc"],
    data = [
        "//xrtl/testing/testdata:image_buffer_data",
    ],
    deps = [
        ":file_util",
        ":image_buffer",
        "//xrtl/testing:gtest_main",
        "//xrtl/testing/diffing:image_differ",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
