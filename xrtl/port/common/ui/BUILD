# Description:
#  Common port variants of //xrtl/ui/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "timer_display_link",
    srcs = ["timer_display_link.cc"],
    hdrs = ["timer_display_link.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:system_clock",
        "//xrtl/base/threading:message_loop",
        "//xrtl/base/threading:thread",
        "//xrtl/ui:display_link",
    ],
)
