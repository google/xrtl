# Description:
#  Windows port variants of //xrtl/testing/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "win32_demo_main",
    srcs = ["win32_demo_main.cc"],
    linkopts = [
        "-Wl,/SUBSYSTEM:WINDOWS",
        "-Wl,ole32.lib",
        "-Wl,shell32.lib",
    ],
    deps = [
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
        "//xrtl/port/windows/base:windows_headers",
        "//xrtl/testing:demo_main_hdrs",
    ],
)
