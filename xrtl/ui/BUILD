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
        ":input_events",
        "//xrtl/base:geometry",
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
    visibility = ["//xrtl/port:ports_only"],
    deps = [
        ":input_events",
        "//xrtl/base:geometry",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/gfx:color",
    ],
)

cc_test(
    name = "control_test",
    srcs = ["control_test.cc"],
    tags = [
        "exclusive",
        "requires_gui",
    ],
    deps = [
        ":control",
        "//xrtl/base:debugging",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:semaphore",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:gtest_main",
        "//xrtl/ui/testing:mock_control_listener",
    ],
)

cc_library(
    name = "display_link",
    hdrs = ["display_link.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "imgui_overlay",
    srcs = ["imgui_overlay.cc"],
    hdrs = ["imgui_overlay.h"],
    deps = [
        ":control",
        "//third_party/imgui",
        "//third_party/proggy_fonts:proggy_tiny_embedded",
        "//xrtl/base:macros",
        "//xrtl/base:system_clock",
        "//xrtl/gfx:context",
        "//xrtl/ui/shaders:imgui_overlay_frag",
        "//xrtl/ui/shaders:imgui_overlay_vert",
        "@com_github_gtruc_glm//:glm",
    ],
)

cc_binary(
    name = "imgui_overlay_demo",
    srcs = ["imgui_overlay_demo.cc"],
    deps = [
        ":display_link",
        ":imgui_overlay",
        ":window",
        "//xrtl/base:logging",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/gfx:context",
        "//xrtl/gfx:context_factory",
        "//xrtl/testing:demo_main",
    ],
)

cc_library(
    name = "input_events",
    hdrs = ["input_events.h"],
    deps = [
        ":key_codes",
        "//xrtl/base:geometry",
        "//xrtl/base:macros",
    ],
)

cc_library(
    name = "key_codes",
    hdrs = ["key_codes.h"],
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
    visibility = ["//xrtl/port:ports_only"],
    deps = [
        "//xrtl/base:macros",
    ],
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
    visibility = ["//xrtl/port:ports_only"],
    deps = [
        ":control",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:message_loop",
        "//xrtl/base/threading:wait_handle",
    ],
)

cc_test(
    name = "window_test",
    srcs = ["window_test.cc"],
    tags = [
        "exclusive",
        "requires_gui",
    ],
    deps = [
        ":window",
        "//xrtl/base:debugging",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/testing:gtest_main",
        "//xrtl/ui/testing:mock_control_listener",
    ],
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
