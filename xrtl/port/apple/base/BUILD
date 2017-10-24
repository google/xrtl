# Description:
#  Apple port variants of //xrtl/base/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "darwin_debugging",
    srcs = ["darwin_debugging.cc"],
    deps = [
        "//xrtl/base:debugging_hdrs",
    ],
)

cc_library(
    name = "mach_system_clock",
    srcs = ["mach_system_clock.cc"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:system_clock_hdrs",
    ],
)
