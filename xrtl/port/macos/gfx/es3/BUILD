# Description:
#  OpenGL ES 3.X platform implementation based on CGL.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = ["//xrtl/tools/target_platform:macos"])

objc_library(
    name = "cgl_platform_context",
    srcs = ["cgl_platform_context.cc"],
    hdrs = ["cgl_platform_context.h"],
    copts = [
        "-std=c++11",
        "-x objective-c++",
    ],
    sdk_frameworks = [
        "CoreFoundation",
        "OpenGL",
    ],
    deps = [
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:macros",
        "//xrtl/base:tracing",
        "//xrtl/gfx/es3:es3_platform_context_hdrs",
        "@com_google_absl//absl/base",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
