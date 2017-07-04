# Description:
#  OpenGL ES 3.X platform implementation based on EGL.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "platform_egl",
    linkopts = select({
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:linux": [
            "-lEGL",
        ],
        "//xrtl/tools/target_platform:macos": [],
        "//xrtl/tools/target_platform:windows": [],
    }),
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
            "-Wl,libEGL.lib",
            "-Wl,libGLESv2.lib",
        ],
    }),
    deps = [
        ":egl_strings",
        "//third_party/glad:glad_egl",
        "//xrtl/base:debugging",
        "//xrtl/base:macros",
        "//xrtl/base:tracing",
        "//xrtl/gfx/es3:es3_platform_context_hdrs",
        "//xrtl/tools/target_config",
    ] + select({
        "//xrtl/tools/target_config:swiftshader": [
            "@com_github_google_swiftshader//:swiftshader",
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
