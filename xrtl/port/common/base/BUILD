# Description:
#  Common port variants of //xrtl/base/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "mach_system_clock",
    srcs = ["mach_system_clock.cc"],
    hdrs = ["mach_system_clock.h"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:system_clock_hdrs",
    ],
)

cc_library(
    name = "posix_logging_sink",
    srcs = ["posix_logging_sink.cc"],
    deps = [
        "//xrtl/base:logging_hdrs",
        "//xrtl/base:system_clock",
    ],
)

cc_library(
    name = "posix_system_clock",
    srcs = ["posix_system_clock.cc"],
    hdrs = ["posix_system_clock.h"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:system_clock_hdrs",
    ],
)
