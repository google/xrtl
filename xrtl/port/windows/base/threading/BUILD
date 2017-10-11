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
    name = "win32_message_loop",
    srcs = ["win32_message_loop.cc"],
    linkopts = [
        "-DEFAULTLIB:user32.lib",
    ],
    deps = [
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/base/threading:threading_hdrs",
        "//xrtl/port/windows/base:windows_headers",
        "@com_google_absl//absl/base",
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
    linkopts = [
        "-DEFAULTLIB:winmm.lib",
    ],
    deps = [
        ":win32_wait_handle",
        "//xrtl/base:logging",
        "//xrtl/base:stopwatch",
        "//xrtl/base/threading:thread_base",
        "//xrtl/base/threading:threading_hdrs",
        "//xrtl/port/windows/base:windows_headers",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:fixed_array",
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
