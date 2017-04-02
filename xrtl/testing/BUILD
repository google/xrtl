# Description:
#  Testing utilities.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

# googletest header shim.
# This can be linked in to cc_library rules that require access to the gunit
# headers (such as test utility libraries/etc).
cc_library(
    name = "gtest",
    testonly = 1,
    hdrs = ["gtest.h"],
    deps = [
        "//xrtl/base:flags",
        "//xrtl/base:logging",
        "@gtest//:gtest",
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
        "@gtest//:gtest",
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
    deps = select({
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
    visibility = ["//visibility:private"],
)
