# Description:
#  OpenGL ES 3.X platform implementation based on EGL.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "platform_egl_unused",
)

cc_library(
    name = "platform_egl_android",
    linkopts = [
        "-pie -ldl -lEGL -lGLESv3 -landroid -Wl,--strip-all",
    ],
    alwayslink = 1,
)

cc_library(
    name = "platform_egl_linux",
    linkopts = [
        "-lEGL",
    ],
    alwayslink = 1,
)

alias(
    name = "platform_egl",
    actual = select({
        "//xrtl/tools/target_platform:android": ":platform_egl_android",
        "//xrtl/tools/target_platform:emscripten": ":platform_egl_unused",
        "//xrtl/tools/target_platform:ios": ":platform_egl_unused",
        "//xrtl/tools/target_platform:linux": ":platform_egl_linux",
        "//xrtl/tools/target_platform:macos": ":platform_egl_unused",
        "//xrtl/tools/target_platform:windows": ":platform_egl_unused",
    }),
)

cc_library(
    name = "platform_swiftshader",
    visibility = ["//visibility:private"],
    deps = [
        "@com_github_google_swiftshader//:swiftshader",
    ],
    alwayslink = 1,
)

cc_library(
    name = "egl_platform_context",
    srcs = ["egl_platform_context.cc"],
    hdrs = ["egl_platform_context.h"],
    linkopts = select({
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:linux": [
            "-ldl",
        ],
        "//xrtl/tools/target_platform:macos": [],
        "//xrtl/tools/target_platform:windows": [
            "-DEFAULTLIB:libEGL.lib",
            "-DEFAULTLIB:libGLESv2.lib",
        ],
    }),
    deps = [
        ":egl_strings",
        "//third_party/glad:glad",
        "//third_party/glad:glad_egl",
        "//xrtl/base:debugging",
        "//xrtl/base:macros",
        "//xrtl/base:tracing",
        "//xrtl/gfx/es3:es3_platform_context_hdrs",
        "//xrtl/tools/target_config",
        "@com_google_absl//absl/base",
    ] + select({
        "//xrtl/tools/target_config:swiftshader": [
            ":platform_swiftshader",
        ],
        "//conditions:default": [
            ":platform_egl",
        ],
    }),
)

cc_library(
    name = "egl_strings",
    srcs = ["egl_strings.cc"],
    hdrs = ["egl_strings.h"],
    deps = [
        "//third_party/glad:glad_egl",
    ],
)
