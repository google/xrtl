# Description:
#  Graphics abstraction layer.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "buffer",
    hdrs = ["buffer.h"],
    deps = [
        ":resource",
    ],
)

cc_library(
    name = "color",
    srcs = ["color.cc"],
    hdrs = ["color.h"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:math",
    ],
)

cc_test(
    name = "color_test",
    size = "small",
    srcs = ["color_test.cc"],
    deps = [
        ":color",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "command_buffer",
    srcs = ["command_buffer.cc"],
    hdrs = ["command_buffer.h"],
    deps = [
        ":command_encoder",
        ":framebuffer",
        ":render_pass",
        "//xrtl/base:logging",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "command_encoder",
    hdrs = ["command_encoder.h"],
    deps = [
        ":buffer",
        ":command_fence",
        ":image",
        ":pipeline",
        ":pipeline_layout",
        ":resource_set",
        ":sampler",
        "//xrtl/base:array_view",
        "//xrtl/base:logging",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "command_fence",
    hdrs = ["command_fence.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "context",
    srcs = ["context.cc"],
    hdrs = ["context.h"],
    deps = [
        ":command_buffer",
        ":command_fence",
        ":device",
        ":framebuffer",
        ":image_view",
        ":memory_heap",
        ":pipeline",
        ":pipeline_layout",
        ":pixel_format",
        ":queue_fence",
        ":render_pass",
        ":resource_set",
        ":sampler",
        ":shader_module",
        ":swap_chain",
        "//xrtl/base:array_view",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:event",
        "//xrtl/ui:control",
    ],
)

cc_library(
    name = "port_context_factory",
    visibility = ["//visibility:private"],
    deps = select({
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:linux": [
            "//xrtl/gfx/es3:es3_context_factory",
        ],
        "//xrtl/tools/target_platform:macos": [],
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/gfx/es3:es3_context_factory",
        ],
    }),
)

cc_library(
    name = "context_factory",
    srcs = ["context_factory.cc"],
    hdrs = ["context_factory.h"],
    deps = [
        ":context_factory_hdrs",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
    ] + select({
        # Always use EGL when targeting swiftshader, and otherwise use the
        # port's default implementation.
        "//xrtl/tools/target_config:swiftshader": [
            "//xrtl/gfx/es3:es3_context_factory",
        ],
        "//conditions:default": [
            ":port_context_factory",
        ],
    }),
)

cc_library(
    name = "context_factory_hdrs",
    hdrs = ["context_factory.h"],
    deps = [
        ":context",
        ":device",
        "//xrtl/base:array_view",
        "//xrtl/base:ref_ptr",
    ],
)

cc_test(
    name = "context_factory_test",
    size = "small",
    srcs = ["context_factory_test.cc"],
    deps = [
        ":context_factory",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "device",
    srcs = ["device.cc"],
    hdrs = ["device.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "framebuffer",
    hdrs = ["framebuffer.h"],
    deps = [
        ":image_view",
        ":render_pass",
        "//xrtl/base:array_view",
        "//xrtl/base:geometry",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "image",
    hdrs = ["image.h"],
    deps = [
        ":render_state",
        ":resource",
        "//xrtl/base:geometry",
    ],
)

cc_library(
    name = "image_view",
    hdrs = ["image_view.h"],
    deps = [
        ":image",
        ":pixel_format",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "memory_heap",
    srcs = ["memory_heap.cc"],
    hdrs = ["memory_heap.h"],
    deps = [
        ":buffer",
        ":image",
        ":resource",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "pipeline",
    hdrs = ["pipeline.h"],
    deps = [
        ":pipeline_layout",
        ":render_state",
        ":shader_module",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "pipeline_layout",
    hdrs = ["pipeline_layout.h"],
    deps = [
        ":resource_set_layout",
        "//xrtl/base:array_view",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "pixel_format",
    srcs = ["pixel_format.cc"],
    hdrs = ["pixel_format.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:macros",
    ],
)

cc_test(
    name = "pixel_format_test",
    size = "small",
    srcs = ["pixel_format_test.cc"],
    deps = [
        ":pixel_format",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "queue_fence",
    hdrs = ["queue_fence.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "render_pass",
    hdrs = ["render_pass.h"],
    deps = [
        ":image",
        ":pixel_format",
        ":render_state",
        "//xrtl/base:array_view",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "render_state",
    hdrs = ["render_state.h"],
    deps = [
        ":pixel_format",
        ":vertex_format",
        "//xrtl/base:fixed_vector",
    ],
)

cc_library(
    name = "resource",
    hdrs = ["resource.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "resource_set",
    hdrs = ["resource_set.h"],
    deps = [
        ":buffer",
        ":image",
        ":image_view",
        ":resource_set_layout",
        ":sampler",
        "//xrtl/base:array_view",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "resource_set_layout",
    hdrs = ["resource_set_layout.h"],
    deps = [
        ":render_pass",
        "//xrtl/base:array_view",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "sampler",
    hdrs = ["sampler.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "shader_module",
    hdrs = ["shader_module.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "swap_chain",
    srcs = ["swap_chain.cc"],
    hdrs = ["swap_chain.h"],
    deps = [
        ":command_buffer",
        ":framebuffer",
        ":pixel_format",
        ":queue_fence",
        "//xrtl/base:geometry",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "vertex_format",
    hdrs = ["vertex_format.h"],
    deps = [
        ":pixel_format",
        "//xrtl/base:logging",
        "//xrtl/base:macros",
    ],
)

cc_test(
    name = "vertex_format_test",
    size = "small",
    srcs = ["vertex_format_test.cc"],
    deps = [
        ":vertex_format",
        "//xrtl/testing:gtest_main",
    ],
)
