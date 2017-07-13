# Description:
#  Linux implementations of the UI types.
#  Currently these are all X11 focused, however we may add more window system
#  integrations in the future (wayland/mir/whatever).

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "x11_control",
    srcs = ["x11_control.cc"],
    hdrs = ["x11_control.h"],
    deps = [
        ":x11_display",
        ":x11_headers",
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
        "//xrtl/base/threading:event",
        "//xrtl/port/common/ui:timer_display_link",
        "//xrtl/ui:control_hdrs",
    ],
)

cc_library(
    name = "x11_display",
    srcs = ["x11_display.cc"],
    hdrs = ["x11_display.h"],
    deps = [
        ":x11_headers",
        "//xrtl/base:logging",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/port/common/base/threading:epoll_message_loop",
    ],
)

cc_test(
    name = "x11_display_test",
    size = "small",
    srcs = ["x11_display_test.cc"],
    tags = ["requires_gui"],
    deps = [
        ":x11_display",
        "//xrtl/testing:gtest_main",
        "//xrtl/tools/target_config",
    ],
)

cc_library(
    name = "x11_headers",
    hdrs = ["x11_headers.h"],
    linkopts = [
        "-lX11",
    ],
    deps = [
        # TODO(benvanik): pull headers in so we can cross-compile?
        # "//third_party/Xorg:libX11",
    ],
)

cc_library(
    name = "x11_system_theme",
    srcs = ["x11_system_theme.cc"],
    deps = [
        ":x11_headers",
        "//xrtl/ui:system_theme_hdrs",
    ],
)

cc_library(
    name = "x11_window",
    srcs = ["x11_window.cc"],
    hdrs = ["x11_window.h"],
    deps = [
        ":x11_control",
        ":x11_display",
        ":x11_headers",
        "//xrtl/base:logging",
        "//xrtl/ui:window_hdrs",
    ],
)
