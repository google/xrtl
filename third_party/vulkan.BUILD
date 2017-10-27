# Description:
#  Vulkan headers, ICD, and layers.
#
#  Depending on :vulkan_headers and/or :vulkan_headers_cpp will allow
#  compilation of C/C++ code against the Vulkan API. Depending on :vulkan allows
#  linking with the loader components.
#
#  Additionally linking to :layers makes the standard Vulkan layers available
#  for use.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE.txt"])

COMMON_COPTS = [
    "-DVULKAN_NON_CMAKE_BUILD",
    # DO NOT SUBMIT linux
    "-DVK_USE_PLATFORM_XLIB_KHR",
    "-DHAVE_SECURE_GETENV",
    "-Wno-unused-const-variable",
]

PLATFORM_DEFINES = [
    # DO NOT SUBMIT Linux
    "VK_USE_PLATFORM_XLIB_KHR",
]

COMMON_LINKOPTS = [
    # DO NOT SUBMIT linux
    "-lpthread",
    "-ldl",
    "-lX11",
]

LAYER_LINKOPTS = COMMON_LINKOPTS + [
    "-Wl,-Bsymbolic,--exclude-libs,ALL",
]

cc_library(
    name = "vulkan_headers",
    hdrs = [
        "include/vulkan/vk_icd.h",
        "include/vulkan/vk_layer.h",
        "include/vulkan/vk_platform.h",
        "include/vulkan/vk_sdk_platform.h",
        "include/vulkan/vulkan.h",
    ],
    copts = COMMON_COPTS,
    defines = PLATFORM_DEFINES,
    # This allows source files to do #include <vulkan/vulkan.h>.
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "vulkan_headers_cpp",
    hdrs = ["include/vulkan/vulkan.hpp"],
    copts = COMMON_COPTS,
    # This allows source files to do #include <vulkan/vulkan.hpp>.
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = [":vulkan_headers"],
)

py_binary(
    name = "lvl_genvk",
    srcs = [
        "scripts/cgenerator.py",
        "scripts/dispatch_table_helper_generator.py",
        "scripts/generator.py",
        "scripts/helper_file_generator.py",
        "scripts/loader_extension_generator.py",
        "scripts/lvl_genvk.py",
        "scripts/object_tracker_generator.py",
        "scripts/parameter_validation_generator.py",
        "scripts/reg.py",
        "scripts/threading_generator.py",
        "scripts/unique_objects_generator.py",
    ],
    main = "scripts/lvl_genvk.py",
    visibility = ["//visibility:private"],
)

load("@//third_party:vulkan_defs.bzl", "generate_vulkan_sources")

generate_vulkan_sources(
    name = "generated_headers",
    outs = [
        "object_tracker.cpp",
        "parameter_validation.cpp",
        "thread_check.h",
        "unique_objects_wrappers.h",
        "vk_dispatch_table_helper.h",
        "vk_enum_string_helper.h",
        "vk_extension_helper.h",
        "vk_layer_dispatch_table.h",
        "vk_loader_extensions.c",
        "vk_loader_extensions.h",
        "vk_object_types.h",
        "vk_safe_struct.cpp",
        "vk_safe_struct.h",
        "vk_struct_size_helper.c",
        "vk_struct_size_helper.h",
    ],
)

cc_library(
    name = "trampolines",
    srcs = [
        "loader/dev_ext_trampoline.c",
        "loader/loader.h",
        "loader/phys_dev_ext.c",
        "loader/unknown_ext_chain.c",
        "loader/vk_loader_platform.h",
    ],
    copts = COMMON_COPTS + [
        "-std=c99",
        "-O3",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":generated_headers",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

# Headers used by both the loader itself and layers.
cc_library(
    name = "vulkan_loader_headers",
    hdrs = [
        "loader/vk_loader_layer.h",
        "loader/vk_loader_platform.h",
    ],
    copts = COMMON_COPTS,
)

cc_library(
    name = "vulkan",
    srcs = [
        "loader/cJSON.c",
        "loader/debug_report.c",
        "loader/extension_manual.c",
        "loader/loader.c",
        "loader/murmurhash.c",
        "loader/trampoline.c",
        "loader/wsi.c",
    ],
    hdrs = [
        "loader/cJSON.h",
        "loader/debug_report.h",
        "loader/extension_manual.h",
        "loader/gpa_helper.h",
        "loader/loader.h",
        "loader/murmurhash.h",
        "loader/wsi.h",
    ],
    copts = COMMON_COPTS + [
        "-DAPI_NAME=Vulkan",
        # DO NOT SUBMIT: figure out what these should be
        "-DFALLBACK_CONFIG_DIRS=\\\"/etc/xdg\\\"",
        "-DFALLBACK_DATA_DIRS=\\\"/usr/local/share:/usr/share\\\"",
        "-DSYSCONFDIR=\\\"/etc\\\"",
        "-DEXTRASYSCONFDIR=\\\"external/com_github_khronosgroup_vulkan/manifests\\\"",
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
        "-std=c99",
    ],
    defines = PLATFORM_DEFINES,
    linkopts = COMMON_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":generated_headers",
        ":trampolines",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

cc_library(
    name = "vk_layer_common",
    srcs = [
        "layers/vk_format_utils.cpp",
        "layers/vk_layer_config.cpp",
        "layers/vk_layer_extension_utils.cpp",
        "layers/vk_layer_table.cpp",
        "layers/vk_layer_utils.cpp",
    ],
    hdrs = [
        "layers/vk_format_utils.h",
        "layers/vk_layer_config.h",
        "layers/vk_layer_data.h",
        "layers/vk_layer_extension_utils.h",
        "layers/vk_layer_logging.h",
        "layers/vk_layer_table.h",
        "layers/vk_layer_utils.h",
        "layers/vk_validation_error_messages.h",
    ],
    copts = COMMON_COPTS + [
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
    ],
    linkopts = LAYER_LINKOPTS,
    deps = [
        ":generated_headers",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

cc_binary(
    name = "libVkLayer_core_validation.so",
    srcs = [
        "layers/buffer_validation.cpp",
        "layers/buffer_validation.h",
        "layers/core_validation.cpp",
        "layers/core_validation.h",
        "layers/core_validation_error_enums.h",
        "layers/core_validation_types.h",
        "layers/descriptor_sets.cpp",
        "layers/descriptor_sets.h",
        "layers/shader_validation.cpp",
        "layers/shader_validation.h",
    ],
    copts = COMMON_COPTS + [
        "-Wno-implicit-fallthrough",  # For layers/descriptor_sets.cpp
        "-Iexternal/com_github_khronosgroup_glslang",
        "-Iexternal/spirv_tools/include",
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
    ],
    linkopts = LAYER_LINKOPTS,
    linkshared = 1,
    deps = [
        ":generated_headers",
        ":vk_layer_common",
        ":vulkan_headers",
        ":vulkan_loader_headers",
        "@com_github_khronosgroup_glslang//:SPIRV",
        "@spirv_tools//:spirv_tools",
    ],
)

cc_binary(
    name = "libVkLayer_object_tracker.so",
    srcs = [
        "layers/object_tracker.h",
        "layers/object_tracker_utils.cpp",
        ":object_tracker.cpp",
    ],
    copts = [
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
        "-Iexternal/com_github_khronosgroup_vulkan/layers",
    ],
    linkopts = LAYER_LINKOPTS,
    linkshared = 1,
    deps = [
        ":generated_headers",
        ":vk_layer_common",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

cc_binary(
    name = "libVkLayer_threading.so",
    srcs = [
        "layers/threading.cpp",
        "layers/threading.h",
        "thread_check.h",
    ],
    copts = COMMON_COPTS + [
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
    ],
    linkopts = LAYER_LINKOPTS,
    linkshared = 1,
    deps = [
        ":generated_headers",
        ":vk_layer_common",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

cc_binary(
    name = "libVkLayer_unique_objects.so",
    srcs = [
        "layers/unique_objects.cpp",
        "layers/unique_objects.h",
        "unique_objects_wrappers.h",
    ],
    copts = COMMON_COPTS + [
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
    ],
    linkopts = LAYER_LINKOPTS,
    linkshared = 1,
    deps = [
        ":generated_headers",
        ":vk_layer_common",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

cc_binary(
    name = "libVkLayer_parameter_validation.so",
    srcs = [
        "layers/parameter_name.h",
        "layers/parameter_validation.h",
        "layers/parameter_validation_utils.cpp",
        ":parameter_validation.cpp",
    ],
    copts = COMMON_COPTS + [
        "-Iexternal/com_github_khronosgroup_vulkan/layers",
        "-Iexternal/com_github_khronosgroup_vulkan/loader",
    ],
    linkopts = LAYER_LINKOPTS,
    linkshared = 1,
    deps = [
        ":generated_headers",
        ":vk_layer_common",
        ":vulkan_headers",
        ":vulkan_loader_headers",
    ],
)

load("@//third_party:vulkan_defs.bzl", "symlink_files")

symlink_files(
    name = "explicit_layers_dir",
    # DO NOT SUBMIT LINUX
    srcs = glob(["layers/linux/**"]) + [
        ":libVkLayer_core_validation.so",
        ":libVkLayer_object_tracker.so",
        ":libVkLayer_threading.so",
        ":libVkLayer_unique_objects.so",
        ":libVkLayer_parameter_validation.so",
    ],
    out = "manifests/vulkan/explicit_layer.d",
)

cc_library(
    name = "layers",
    data = [":explicit_layers_dir"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "vulkaninfo",
    srcs = ["demos/vulkaninfo.c"],
    copts = COMMON_COPTS + [
        "-std=c99",
    ],
    linkopts = COMMON_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":layers",
        ":vulkan",
        ":vulkan_headers",
    ],
)
