# Description:
#  Platform user interface and utilities.
#  The types here provide a consistent set of APIs for using the windowing
#  systems on various platforms, either directly as top-level windows or
#  embedded within existing views as Controls.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "control",
    srcs = ["control.cc"],
    hdrs = ["control.h"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/gfx:color",
    ] + select({
        "//xrtl/tools/target_platform:linux": [
            "//xrtl/port/linux/ui:x11_control",
        ],
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/ui:win32_control",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "control_hdrs",
    hdrs = ["control.h"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/gfx:color",
    ],
    visibility = ["//xrtl/port:ports_only"],
)

cc_test(
    name = "control_test",
    srcs = ["control_test.cc"],
    deps = [
        ":control",
        "//xrtl/base:debugging",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:semaphore",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:gtest_main",
        "//xrtl/ui/testing:mock_control_listener",
    ],
    tags = ["exclusive"],
)

cc_library(
    name = "system_theme",
    hdrs = ["system_theme.h"],
    deps = [
        "//xrtl/base:macros",
    ] + select({
        "//xrtl/tools/target_platform:linux": [
            "//xrtl/port/linux/ui:x11_system_theme",
        ],
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/ui:win32_system_theme",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "system_theme_hdrs",
    hdrs = ["system_theme.h"],
    deps = [
        "//xrtl/base:macros",
    ],
    visibility = ["//xrtl/port:ports_only"],
)

cc_test(
    name = "system_theme_test",
    srcs = ["system_theme_test.cc"],
    deps = [
        ":system_theme",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "window",
    hdrs = ["window.h"],
    deps = [
        ":control",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/base/threading:wait_handle",
    ] + select({
        "//xrtl/tools/target_platform:linux": [
            "//xrtl/port/linux/ui:x11_window",
        ],
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/ui:win32_window",
        ],
        "//conditions:default": [],
    }),
)

cc_library(
    name = "window_hdrs",
    hdrs = ["window.h"],
    deps = [
        ":control",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/base/threading:wait_handle",
    ],
    visibility = ["//xrtl/port:ports_only"],
)

cc_binary(
    name = "window_demo",
    srcs = ["window_demo.cc"],
    deps = [
        ":window",
        "//xrtl/base:logging",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:demo_main",
    ],
)

cc_binary(
    name = "window_pong_demo",
    srcs = ["window_pong_demo.cc"],
    deps = [
        ":window",
        "//xrtl/base:logging",
        "//xrtl/base:math",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:demo_main",
    ],
)

cc_test(
    name = "window_test",
    srcs = ["window_test.cc"],
    deps = [
        ":window",
        "//xrtl/base:debugging",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:gtest_main",
        "//xrtl/ui/testing:mock_control_listener",
    ],
    tags = ["exclusive"],
)
