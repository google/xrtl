# Description:
#  Win32 implementations of the UI types.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = ["//xrtl/tools/target_platform:windows"])

cc_library(
    name = "win32_control",
    srcs = ["win32_control.cc"],
    hdrs = ["win32_control.h"],
    linkopts = [
        "-DEFAULTLIB:wtsapi32.lib",
    ],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base/threading:event",
        "//xrtl/port/common/ui:timer_display_link",
        "//xrtl/port/windows/base:windows_headers",
        "//xrtl/ui:control_hdrs",
        "@com_google_absl//absl/base",
    ],
)

cc_library(
    name = "win32_system_theme",
    srcs = ["win32_system_theme.cc"],
    deps = [
        "//xrtl/port/windows/base:windows_headers",
        "//xrtl/ui:system_theme_hdrs",
    ],
)

cc_library(
    name = "win32_window",
    srcs = ["win32_window.cc"],
    hdrs = ["win32_window.h"],
    deps = [
        ":win32_control",
        "//xrtl/base:logging",
        "//xrtl/port/windows/base:windows_headers",
        "//xrtl/ui:window_hdrs",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
