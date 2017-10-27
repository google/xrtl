# Description:
#  Vulkan graphics backend.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

# cc_library(
#     name = "vk_buffer",
#     srcs = ["vk_buffer.cc"],
#     hdrs = ["vk_buffer.h"],
#     deps = [
#         ":vk_common",
#         ":vk_platform_context",
#         ":vk_queue_object",
#         "//xrtl/base:tracing",
#         "//xrtl/gfx:buffer",
#         "//xrtl/gfx:context",
#         "//xrtl/gfx:memory_heap",
#     ],
# )

# cc_library(
#     name = "vk_command_buffer",
#     srcs = ["vk_command_buffer.cc"],
#     hdrs = ["vk_command_buffer.h"],
#     deps = [
#         ":vk_command_encoder",
#         ":vk_common",
#         "//xrtl/gfx:command_buffer",
#     ],
# )

# cc_library(
#     name = "vk_command_encoder",
#     srcs = ["vk_command_encoder.cc"],
#     hdrs = ["vk_command_encoder.h"],
#     deps = [
#         ":vk_buffer",
#         ":vk_common",
#         ":vk_image",
#         ":vk_pipeline",
#         ":vk_sampler",
#         "//xrtl/base:debugging",
#         "//xrtl/gfx:command_encoder",
#         "//xrtl/gfx:framebuffer",
#         "@com_google_absl//absl/container:inlined_vector",
#     ],
# )

# cc_library(
#     name = "vk_command_fence",
#     hdrs = ["vk_command_fence.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:command_fence",
#     ],
# )

cc_library(
    name = "vk_common",
    srcs = ["vk_common.cc"],
    hdrs = ["vk_common.h"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_github_khronosgroup_vulkan//:vulkan",
        "//xrtl/base:logging",
        # TODO(benvanik): make conditional on debug mode?
        "@com_github_khronosgroup_vulkan//:layers",
    ],
)

# cc_library(
#     name = "vk_context",
#     srcs = ["vk_context.cc"],
#     hdrs = ["vk_context.h"],
#     deps = [
#         ":vk_buffer",
#         ":vk_command_buffer",
#         ":vk_command_fence",
#         ":vk_common",
#         ":vk_framebuffer",
#         ":vk_image",
#         ":vk_image_view",
#         ":vk_memory_heap",
#         ":vk_pipeline",
#         ":vk_pipeline_layout",
#         ":vk_platform_context",
#         ":vk_program",
#         ":vk_queue",
#         ":vk_queue_fence",
#         ":vk_render_pass",
#         ":vk_resource_set",
#         ":vk_resource_set_layout",
#         ":vk_sampler",
#         ":vk_shader",
#         ":vk_shader_module",
#         ":vk_swap_chain",
#         "//xrtl/base:flags",
#         "//xrtl/base:tracing",
#         "//xrtl/base/threading:thread",
#         "//xrtl/gfx:context",
#         "//xrtl/gfx:context_factory_hdrs",
#         "//xrtl/gfx/util:memory_command_buffer",
#         "//xrtl/gfx/util:memory_command_decoder",
#     ],
# )

cc_library(
    name = "vk_context_factory",
    srcs = ["vk_context_factory.cc"],
    hdrs = ["vk_context_factory.h"],
    defines = [
        "XRTL_HAS_GFX_VULKAN=1",
    ],
    deps = [
        ":vk_common",
        #":vk_context",
        ":vk_device",
        #":vk_platform_context",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
        "//xrtl/gfx:context_factory_hdrs",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "vk_device",
    srcs = ["vk_device.cc"],
    hdrs = ["vk_device.h"],
    deps = [
        ":vk_common",
        "//xrtl/gfx:device",
    ],
)

# cc_library(
#     name = "vk_framebuffer",
#     srcs = ["vk_framebuffer.cc"],
#     hdrs = ["vk_framebuffer.h"],
#     deps = [
#         ":vk_common",
#         ":vk_queue_object",
#         "//xrtl/gfx:framebuffer",
#     ],
# )

# cc_library(
#     name = "vk_image",
#     srcs = ["vk_image.cc"],
#     hdrs = ["vk_image.h"],
#     deps = [
#         ":vk_common",
#         ":vk_image_view",
#         ":vk_pixel_format",
#         ":vk_platform_context",
#         ":vk_queue_object",
#         "//xrtl/base:tracing",
#         "//xrtl/gfx:context",
#         "//xrtl/gfx:image",
#         "//xrtl/gfx:memory_heap",
#     ],
# )

# cc_library(
#     name = "vk_image_view",
#     hdrs = ["vk_image_view.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:image_view",
#     ],
# )

# cc_library(
#     name = "vk_memory_heap",
#     srcs = ["vk_memory_heap.cc"],
#     hdrs = ["vk_memory_heap.h"],
#     deps = [
#         ":vk_buffer",
#         ":vk_common",
#         ":vk_image",
#         ":vk_pixel_format",
#         ":vk_platform_context",
#         ":vk_queue_object",
#         "//xrtl/base:math",
#         "//xrtl/base:tracing",
#         "//xrtl/gfx:memory_heap",
#     ],
# )

# cc_library(
#     name = "vk_pipeline",
#     srcs = ["vk_pipeline.cc"],
#     hdrs = ["vk_pipeline.h"],
#     deps = [
#         ":vk_common",
#         ":vk_program",
#         ":vk_queue_object",
#         "//xrtl/gfx:pipeline",
#     ],
# )

# cc_library(
#     name = "vk_pipeline_layout",
#     hdrs = ["vk_pipeline_layout.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:pipeline_layout",
#     ],
# )

# cc_library(
#     name = "vk_pixel_format",
#     srcs = ["vk_pixel_format.cc"],
#     hdrs = ["vk_pixel_format.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:pixel_format",
#     ],
# )

# cc_library(
#     name = "port_platform_context",
#     visibility = ["//visibility:private"],
#     deps = select({
#         "//xrtl/tools/target_platform:android": [],
#         "//xrtl/tools/target_platform:emscripten": [],
#         "//xrtl/tools/target_platform:ios": [],
#         "//xrtl/tools/target_platform:linux": [
#             "//xrtl/port/common/gfx/vk:egl_platform_context",
#         ],
#         "//xrtl/tools/target_platform:macos": [
#             "//xrtl/port/macos/gfx/vk:cgl_platform_context",
#         ],
#         "//xrtl/tools/target_platform:windows": [
#             "//xrtl/port/windows/gfx/vk:wgl_platform_context",
#         ],
#     }),
# )

# cc_library(
#     name = "vk_platform_context",
#     srcs = ["vk_platform_context.cc"],
#     hdrs = ["vk_platform_context.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/base:flags",
#         "//xrtl/base:geometry",
#         "//xrtl/base:ref_ptr",
#         "//xrtl/base:tracing",
#         "//xrtl/base/threading:thread",
#         "@com_google_absl//absl/strings",
#     ] + select({
#         # Always use EGL when targeting swiftshader, and otherwise use the
#         # port's default implementation.
#         "//xrtl/tools/target_config:swiftshader": [
#             "//xrtl/port/common/gfx/vk:egl_platform_context",
#         ],
#         "//conditions:default": [
#             ":port_platform_context",
#         ],
#     }),
# )

# cc_library(
#     name = "vk_platform_context_hdrs",
#     hdrs = ["vk_platform_context.h"],
#     visibility = ["//xrtl/port:ports_only"],
#     deps = [
#         ":vk_common",
#         "//xrtl/base:geometry",
#         "//xrtl/base:ref_ptr",
#         "@com_google_absl//absl/strings",
#     ],
# )

# cc_library(
#     name = "vk_program",
#     srcs = ["vk_program.cc"],
#     hdrs = ["vk_program.h"],
#     deps = [
#         ":vk_common",
#         ":vk_platform_context",
#         ":vk_shader",
#         "//xrtl/base:tracing",
#         "@com_google_absl//absl/container:inlined_vector",
#         "@com_google_absl//absl/types:span",
#     ],
# )

# cc_library(
#     name = "vk_queue",
#     srcs = ["vk_queue.cc"],
#     hdrs = ["vk_queue.h"],
#     deps = [
#         ":vk_command_buffer",
#         ":vk_common",
#         ":vk_platform_context",
#         ":vk_queue_fence",
#         ":vk_queue_object",
#         "//xrtl/base:intrusive_list",
#         "//xrtl/base:intrusive_pool",
#         "//xrtl/base:tracing",
#         "//xrtl/base/threading:event",
#         "//xrtl/base/threading:thread",
#         "//xrtl/gfx:command_buffer",
#         "//xrtl/gfx:queue_fence",
#         "//xrtl/gfx/util:memory_command_buffer",
#         "//xrtl/gfx/util:memory_command_decoder",
#         "@com_google_absl//absl/container:inlined_vector",
#         "@com_google_absl//absl/types:span",
#     ],
# )

# cc_library(
#     name = "vk_queue_fence",
#     srcs = ["vk_queue_fence.cc"],
#     hdrs = ["vk_queue_fence.h"],
#     deps = [
#         ":vk_common",
#         ":vk_platform_context",
#         ":vk_queue_object",
#         "//xrtl/base:system_clock",
#         "//xrtl/base:tracing",
#         "//xrtl/base/threading:event",
#         "//xrtl/base/threading:thread",
#         "//xrtl/gfx:queue_fence",
#     ],
# )

# cc_library(
#     name = "vk_queue_object",
#     hdrs = ["vk_queue_object.h"],
#     deps = [
#         ":vk_platform_context",
#         "//xrtl/base:ref_ptr",
#     ],
# )

# cc_library(
#     name = "vk_render_pass",
#     hdrs = ["vk_render_pass.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:render_pass",
#     ],
# )

# cc_library(
#     name = "vk_resource_set",
#     hdrs = ["vk_resource_set.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:resource_set",
#     ],
# )

# cc_library(
#     name = "vk_resource_set_layout",
#     hdrs = ["vk_resource_set_layout.h"],
#     deps = [
#         ":vk_common",
#         "//xrtl/gfx:resource_set_layout",
#     ],
# )

# cc_library(
#     name = "vk_sampler",
#     srcs = ["vk_sampler.cc"],
#     hdrs = ["vk_sampler.h"],
#     deps = [
#         ":vk_common",
#         ":vk_platform_context",
#         ":vk_queue_object",
#         "//xrtl/gfx:sampler",
#     ],
# )

# cc_library(
#     name = "vk_shader",
#     srcs = ["vk_shader.cc"],
#     hdrs = ["vk_shader.h"],
#     deps = [
#         ":vk_common",
#         ":vk_platform_context",
#         "//xrtl/base:debugging",
#         "//xrtl/base:ref_ptr",
#         "//xrtl/base:tracing",
#         "@com_google_absl//absl/strings",
#         "@com_google_absl//absl/types:span",
#         "@spirv_cross//:spirv_cross_lib",
#     ],
# )

# cc_library(
#     name = "vk_shader_module",
#     srcs = ["vk_shader_module.cc"],
#     hdrs = ["vk_shader_module.h"],
#     deps = [
#         ":vk_common",
#         ":vk_queue_object",
#         ":vk_shader",
#         "//xrtl/gfx:shader_module",
#     ],
# )

# cc_library(
#     name = "vk_swap_chain",
#     srcs = ["vk_swap_chain.cc"],
#     hdrs = ["vk_swap_chain.h"],
#     deps = [
#         ":vk_common",
#         ":vk_image",
#         ":vk_platform_context",
#         ":vk_queue",
#         ":vk_queue_fence",
#         "//xrtl/base:system_clock",
#         "//xrtl/base:tracing",
#         "//xrtl/base/threading:event",
#         "//xrtl/base/threading:semaphore",
#         "//xrtl/base/threading:thread",
#         "//xrtl/gfx:memory_heap",
#         "//xrtl/gfx:swap_chain",
#         "//xrtl/ui:control",
#     ],
# )
