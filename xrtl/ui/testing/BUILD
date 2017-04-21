# Description:
#  Testing utilities for UI code.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "mock_control_listener",
    hdrs = ["mock_control_listener.h"],
    deps = [
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:semaphore",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:gtest_main",
        "//xrtl/ui:control",
    ],
    testonly = 1,
)
