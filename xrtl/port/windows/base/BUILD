# Description:
#  Windows port variants of //xrtl/base/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = ["//xrtl/tools/target_platform:windows"])

cc_library(
    name = "win32_debugging",
    srcs = ["win32_debugging.cc"],
    deps = [
        ":windows_headers",
        "//xrtl/base:debugging_hdrs",
        "//xrtl/base:logging",
    ],
)

cc_library(
    name = "win32_env",
    srcs = ["win32_env.cc"],
    deps = [
        ":windows_headers",
        "//xrtl/base:env_hdrs",
        "//xrtl/base:logging",
    ],
)

cc_library(
    name = "win32_system_clock",
    srcs = ["win32_system_clock.cc"],
    deps = [
        ":windows_headers",
        "//xrtl/base:macros",
        "//xrtl/base:system_clock_hdrs",
    ],
)

cc_library(
    name = "windows_headers",
    hdrs = ["windows.h"],
    copts = [
        "/UNOGDI",
        "/wd9025",
    ],
    defines = [
        "UNICODE=1",
    ],
    visibility = ["//xrtl/port:windows_only"],
    deps = [
        "//xrtl/tools/target_platform",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
