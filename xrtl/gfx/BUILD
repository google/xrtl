# Description:
#  Graphics abstraction layer.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "portable_cc_test")

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

portable_cc_test(
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
        ":managed_object",
        ":render_pass",
        "//xrtl/base:logging",
        "//xrtl/base:macros",
        "@com_google_absl//absl/types:span",
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
        "//xrtl/base:logging",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "command_fence",
    hdrs = ["command_fence.h"],
    deps = [
        ":managed_object",
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
        ":resource_set_layout",
        ":sampler",
        ":shader_module",
        ":swap_chain",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:event",
        "//xrtl/ui:control",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "port_context_factory",
    visibility = ["//visibility:private"],
    deps = select(
        {
            # "//xrtl/tools/target_platform:android": [],
            # "//xrtl/tools/target_platform:emscripten": [],
            # "//xrtl/tools/target_platform:ios": [],
            "//xrtl/tools/target_platform:linux": [
                "//xrtl/gfx/es3:es3_context_factory",
                "//xrtl/gfx/vk:vk_context_factory",
            ],
            "//xrtl/tools/target_platform:macos": [
                "//xrtl/gfx/es3:es3_context_factory",
            ],
            "//xrtl/tools/target_platform:windows": [
                "//xrtl/gfx/es3:es3_context_factory",
            ],
        },
        no_match_error = "No graphics context support on this platform",
    ),
)

cc_library(
    name = "context_factory",
    srcs = ["context_factory.cc"],
    hdrs = ["context_factory.h"],
    deps = [
        ":context",
        ":device",
        "@com_google_absl//absl/types:span",
        "//xrtl/base:ref_ptr",
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
        "//xrtl/base:ref_ptr",
        "@com_google_absl//absl/types:span",
    ],
)

portable_cc_test(
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
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "framebuffer",
    hdrs = ["framebuffer.h"],
    deps = [
        ":image_view",
        ":managed_object",
        ":render_pass",
        "//xrtl/base:geometry",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/types:span",
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
        ":managed_object",
        ":pixel_format",
    ],
)

cc_library(
    name = "managed_object",
    hdrs = ["managed_object.h"],
    deps = [
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
        ":managed_object",
        ":resource",
        "//xrtl/base:macros",
    ],
)

cc_library(
    name = "pipeline",
    hdrs = ["pipeline.h"],
    deps = [
        ":managed_object",
        ":pipeline_layout",
        ":render_state",
        ":shader_module",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "pipeline_layout",
    hdrs = ["pipeline_layout.h"],
    deps = [
        ":managed_object",
        ":resource_set_layout",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/types:span",
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

portable_cc_test(
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
        ":managed_object",
    ],
)

cc_library(
    name = "render_pass",
    hdrs = ["render_pass.h"],
    deps = [
        ":image",
        ":managed_object",
        ":pixel_format",
        ":render_state",
        "//xrtl/base:macros",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "render_state",
    hdrs = ["render_state.h"],
    deps = [
        ":pixel_format",
        ":vertex_format",
        "@com_google_absl//absl/container:inlined_vector",
    ],
)

cc_library(
    name = "resource",
    hdrs = ["resource.h"],
    deps = [
        ":managed_object",
    ],
)

cc_library(
    name = "resource_set",
    hdrs = ["resource_set.h"],
    deps = [
        ":buffer",
        ":image",
        ":image_view",
        ":managed_object",
        ":resource_set_layout",
        ":sampler",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "resource_set_layout",
    hdrs = ["resource_set_layout.h"],
    deps = [
        ":managed_object",
        ":render_pass",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "sampler",
    hdrs = ["sampler.h"],
    deps = [
        ":managed_object",
    ],
)

cc_library(
    name = "shader_module",
    hdrs = ["shader_module.h"],
    deps = [
        ":managed_object",
    ],
)

cc_library(
    name = "swap_chain",
    srcs = ["swap_chain.cc"],
    hdrs = ["swap_chain.h"],
    deps = [
        ":command_buffer",
        ":framebuffer",
        ":image",
        ":image_view",
        ":managed_object",
        ":pixel_format",
        ":queue_fence",
        "//xrtl/base:geometry",
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

portable_cc_test(
    name = "vertex_format_test",
    size = "small",
    srcs = ["vertex_format_test.cc"],
    deps = [
        ":vertex_format",
        "//xrtl/testing:gtest_main",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
