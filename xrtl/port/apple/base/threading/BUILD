# Description:
#  CFRunLoop and Mach port variants of //xrtl/base/threading/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

objc_library(
    name = "cfrunloop_message_loop",
    srcs = ["cfrunloop_message_loop.mm"],
    copts = [
        "-std=c++11",
        "-x objective-c++",
    ],
    sdk_frameworks = ["CoreFoundation"],
    deps = [
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/base/threading:threading_hdrs",
    ],
)
