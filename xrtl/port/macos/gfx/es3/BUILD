# Description:
#  OpenGL ES 3.X platform implementation based on CGL.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

objc_library(
    name = "cgl_platform_context",
    srcs = ["cgl_platform_context.cc"],
    hdrs = ["cgl_platform_context.h"],
    copts = [
        "-std=c++11",
        "-x objective-c++",
    ],
    sdk_frameworks = [
        "CoreFoundation",
        "OpenGL",
    ],
    deps = [
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:macros",
        "//xrtl/base:tracing",
        "//xrtl/gfx/es3:es3_platform_context_hdrs",
        "@com_google_absl//absl/base",
    ],
)
