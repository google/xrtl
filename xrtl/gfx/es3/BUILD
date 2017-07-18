# Description:
#  OpenGL ES 3.X graphics backend.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "es3_buffer",
    srcs = ["es3_buffer.cc"],
    hdrs = ["es3_buffer.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        "//xrtl/gfx:buffer",
        "//xrtl/gfx:memory_heap",
    ],
)

cc_library(
    name = "es3_command_buffer",
    srcs = ["es3_command_buffer.cc"],
    hdrs = ["es3_command_buffer.h"],
    deps = [
        ":es3_command_encoder",
        ":es3_common",
        "//xrtl/gfx:command_buffer",
    ],
)

cc_library(
    name = "es3_command_encoder",
    srcs = ["es3_command_encoder.cc"],
    hdrs = ["es3_command_encoder.h"],
    deps = [
        ":es3_buffer",
        ":es3_common",
        ":es3_image",
        ":es3_pipeline",
        ":es3_sampler",
        "//xrtl/gfx:command_encoder",
        "//xrtl/gfx:framebuffer",
    ],
)

cc_library(
    name = "es3_command_fence",
    hdrs = ["es3_command_fence.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:command_fence",
    ],
)

cc_library(
    name = "es3_common",
    hdrs = ["es3_common.h"],
    deps = [
        "//third_party/glad",
        "//xrtl/base:logging",
    ],
)

cc_library(
    name = "es3_context",
    srcs = ["es3_context.cc"],
    hdrs = ["es3_context.h"],
    deps = [
        ":es3_command_buffer",
        ":es3_command_fence",
        ":es3_common",
        ":es3_framebuffer",
        ":es3_image_view",
        ":es3_memory_heap",
        ":es3_pipeline",
        ":es3_pipeline_layout",
        ":es3_platform_context",
        ":es3_program",
        ":es3_queue_fence",
        ":es3_render_pass",
        ":es3_resource_set",
        ":es3_resource_set_layout",
        ":es3_sampler",
        ":es3_shader",
        ":es3_shader_module",
        ":es3_swap_chain",
        "//xrtl/base:tracing",
        "//xrtl/gfx:context",
        "//xrtl/gfx:context_factory_hdrs",
        "//xrtl/gfx/util:memory_command_buffer",
        "//xrtl/gfx/util:memory_command_decoder",
    ],
)

cc_library(
    name = "es3_context_hdrs",
    hdrs = ["es3_context.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        "//xrtl/gfx:context",
        "//xrtl/gfx:context_factory_hdrs",
    ],
)

cc_library(
    name = "es3_context_factory",
    srcs = ["es3_context_factory.cc"],
    hdrs = ["es3_context_factory.h"],
    defines = [
        "XRTL_HAS_GFX_OPENGL_ES3=1",
    ],
    deps = [
        ":es3_common",
        ":es3_context",
        ":es3_device",
        ":es3_platform_context",
        "//xrtl/base:logging",
        "//xrtl/gfx:context_factory_hdrs",
    ],
)

cc_library(
    name = "es3_device",
    srcs = ["es3_device.cc"],
    hdrs = ["es3_device.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:device",
    ],
)

cc_library(
    name = "es3_framebuffer",
    srcs = ["es3_framebuffer.cc"],
    hdrs = ["es3_framebuffer.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:framebuffer",
    ],
)

cc_library(
    name = "es3_image",
    srcs = ["es3_image.cc"],
    hdrs = ["es3_image.h"],
    deps = [
        ":es3_common",
        ":es3_image_view",
        ":es3_pixel_format",
        ":es3_platform_context",
        "//xrtl/gfx:image",
        "//xrtl/gfx:memory_heap",
    ],
)

cc_library(
    name = "es3_image_view",
    hdrs = ["es3_image_view.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:image_view",
    ],
)

cc_library(
    name = "es3_memory_heap",
    srcs = ["es3_memory_heap.cc"],
    hdrs = ["es3_memory_heap.h"],
    deps = [
        ":es3_buffer",
        ":es3_common",
        ":es3_image",
        ":es3_pixel_format",
        ":es3_platform_context",
        "//xrtl/base:math",
        "//xrtl/base:tracing",
        "//xrtl/gfx:memory_heap",
    ],
)

cc_library(
    name = "es3_pipeline",
    srcs = ["es3_pipeline.cc"],
    hdrs = ["es3_pipeline.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        ":es3_program",
        "//xrtl/gfx:pipeline",
    ],
)

cc_library(
    name = "es3_pipeline_layout",
    hdrs = ["es3_pipeline_layout.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:pipeline_layout",
    ],
)

cc_library(
    name = "es3_pixel_format",
    srcs = ["es3_pixel_format.cc"],
    hdrs = ["es3_pixel_format.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:pixel_format",
    ],
)

cc_library(
    name = "port_platform_context",
    visibility = ["//visibility:private"],
    deps = select({
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:linux": [
            "//xrtl/port/common/gfx/es3:egl_platform_context",
        ],
        "//xrtl/tools/target_platform:macos": [],
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/gfx/es3:wgl_platform_context",
        ],
    }),
)

cc_library(
    name = "es3_platform_context",
    srcs = ["es3_platform_context.cc"],
    hdrs = ["es3_platform_context.h"],
    deps = [
        ":es3_common",
        "//xrtl/base:flags",
        "//xrtl/base:geometry",
        "//xrtl/base:ref_ptr",
        "//xrtl/base/threading:thread",
    ] + select({
        # Always use EGL when targeting swiftshader, and otherwise use the
        # port's default implementation.
        "//xrtl/tools/target_config:swiftshader": [
            "//xrtl/port/common/gfx/es3:egl_platform_context",
        ],
        "//conditions:default": [
            ":port_platform_context",
        ],
    }),
)

cc_library(
    name = "es3_platform_context_hdrs",
    hdrs = ["es3_platform_context.h"],
    visibility = ["//xrtl/port:ports_only"],
    deps = [
        ":es3_common",
        "//xrtl/base:geometry",
        "//xrtl/base:ref_ptr",
    ],
)

cc_library(
    name = "es3_program",
    srcs = ["es3_program.cc"],
    hdrs = ["es3_program.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        ":es3_shader",
        "//xrtl/base:array_view",
        "//xrtl/base:tracing",
    ],
)

cc_library(
    name = "es3_queue",
    srcs = ["es3_queue.cc"],
    hdrs = ["es3_queue.h"],
    deps = [
        ":es3_command_buffer",
        ":es3_common",
        ":es3_platform_context",
        ":es3_queue_fence",
        "//xrtl/base:array_view",
        "//xrtl/base:tracing",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/gfx/util:memory_command_buffer",
        "//xrtl/gfx/util:memory_command_decoder",
    ],
)

cc_library(
    name = "es3_queue_fence",
    srcs = ["es3_queue_fence.cc"],
    hdrs = ["es3_queue_fence.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/gfx:queue_fence",
    ],
)

cc_library(
    name = "es3_render_pass",
    hdrs = ["es3_render_pass.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:render_pass",
    ],
)

cc_library(
    name = "es3_resource_set",
    hdrs = ["es3_resource_set.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:resource_set",
    ],
)

cc_library(
    name = "es3_resource_set_layout",
    hdrs = ["es3_resource_set_layout.h"],
    deps = [
        ":es3_common",
        "//xrtl/gfx:resource_set_layout",
    ],
)

cc_library(
    name = "es3_sampler",
    srcs = ["es3_sampler.cc"],
    hdrs = ["es3_sampler.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        "//xrtl/gfx:sampler",
    ],
)

cc_library(
    name = "es3_shader",
    srcs = ["es3_shader.cc"],
    hdrs = ["es3_shader.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        "//xrtl/base:array_view",
        "//xrtl/base:debugging",
        "//xrtl/base:string_view",
        "//xrtl/base:tracing",
        "@com_github_khronosgroup_spirv_cross//:spirv_cross_lib",
    ],
)

cc_library(
    name = "es3_shader_module",
    srcs = ["es3_shader_module.cc"],
    hdrs = ["es3_shader_module.h"],
    deps = [
        ":es3_common",
        ":es3_platform_context",
        ":es3_shader",
        "//xrtl/gfx:shader_module",
    ],
)

cc_library(
    name = "es3_swap_chain",
    srcs = ["es3_swap_chain.cc"],
    hdrs = ["es3_swap_chain.h"],
    deps = [
        ":es3_common",
        ":es3_image",
        ":es3_platform_context",
        ":es3_queue",
        ":es3_queue_fence",
        "//xrtl/base:system_clock",
        "//xrtl/base:tracing",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:semaphore",
        "//xrtl/base/threading:thread",
        "//xrtl/gfx:memory_heap",
        "//xrtl/gfx:swap_chain",
        "//xrtl/ui:control",
    ],
)
