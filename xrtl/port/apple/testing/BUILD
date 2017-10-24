# Description:
#  Testing utilities for iOS and MacOS.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = [
    "//xrtl/tools/target_platform:ios",
    "//xrtl/tools/target_platform:macos",
])

exports_files(["apple_test.plist"])

objc_library(
    name = "apple_test_main",
    testonly = 1,
    srcs = ["apple_test_main.mm"],
    sdk_frameworks = [
        "XCTest",
    ],
    deps = [
        "//xrtl/base:env",
        "//xrtl/base:flags",
        "//xrtl/testing:file_util",
        "//xrtl/testing:gtest",
        "//xrtl/tools/target_platform",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
