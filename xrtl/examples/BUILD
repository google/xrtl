# Description:
#  Examples exercising large pieces of functionality.
#  Demos that target individual features are available side-by-side with their
#  code (such as window_demo).

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_binary(
    name = "triangle_example",
    srcs = ["triangle_example.cc"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:system_clock",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/gfx:context",
        "//xrtl/gfx:context_factory",
        "//xrtl/gfx/spirv:shader_compiler",
        "//xrtl/testing:demo_main",
        "//xrtl/ui:window",
    ],
)

cc_binary(
    name = "triangle_full_example",
    srcs = ["triangle_full_example.cc"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:system_clock",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/gfx:context",
        "//xrtl/gfx:context_factory",
        "//xrtl/gfx/spirv:shader_compiler",
        "//xrtl/testing:demo_main",
        "//xrtl/ui:window",
    ],
)
