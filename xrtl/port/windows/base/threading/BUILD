# Description:
#  Windows port variants of //xrtl/base/threading/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "win32_event",
    srcs = ["win32_event.cc"],
    deps = [
        ":win32_wait_handle",
        "//xrtl/base/threading:threading_hdrs",
        "//xrtl/port/windows/base:windows_headers",
    ],
)

cc_library(
    name = "win32_semaphore",
    srcs = ["win32_semaphore.cc"],
    deps = [
        ":win32_wait_handle",
        "//xrtl/base/threading:threading_hdrs",
        "//xrtl/port/windows/base:windows_headers",
    ],
)

cc_library(
    name = "win32_thread",
    srcs = ["win32_thread.cc"],
    deps = [
        ":win32_wait_handle",
        "//xrtl/base:logging",
        "//xrtl/base:stopwatch",
        "//xrtl/base/threading:threading_hdrs",
        "//xrtl/port/windows/base:windows_headers",
    ],
    linkopts = [
        "-Wl,winmm.lib",
    ],
)

cc_library(
    name = "win32_wait_handle",
    hdrs = ["win32_wait_handle.h"],
    deps = [
        "//xrtl/base/threading:wait_handle",
        "//xrtl/port/windows/base:windows_headers",
    ],
)
