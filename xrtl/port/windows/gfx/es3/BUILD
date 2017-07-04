# Description:
#  OpenGL ES 3.X platform implementation based on WGL.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "wgl_platform_context",
    srcs = ["wgl_platform_context.cc"],
    hdrs = ["wgl_platform_context.h"],
    linkopts = [
        "-Wl,gdi32.lib",
    ],
    deps = [
        "//third_party/glad:glad_wgl",
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:macros",
        "//xrtl/base:tracing",
        "//xrtl/gfx/es3:es3_platform_context_hdrs",
    ],
)
