# Description:
#  Windows port variants of //xrtl/base/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

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
    name = "win32_system_clock",
    srcs = ["win32_system_clock.cc"],
    hdrs = ["win32_system_clock.h"],
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
