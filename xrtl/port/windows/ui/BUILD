# Description:
#  Win32 implementations of the UI types.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "win32_control",
    srcs = ["win32_control.cc"],
    hdrs = ["win32_control.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base/threading:event",
        "//xrtl/port/common/ui:timer_display_link",
        "//xrtl/port/windows/base:windows_headers",
        "//xrtl/ui:control_hdrs",
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
