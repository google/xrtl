# Description:
#  Testing utilities.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

# demo main() entry point.
# This is linked in to demo cc_binaries.
cc_library(
    name = "demo_main",
    srcs = ["demo_main.cc"],
    hdrs = ["demo_main.h"],
    deps = [
        ":platform_demo_main",
        "//xrtl/base:debugging",
        "//xrtl/base:flags",
        "//xrtl/base:logging",
    ],
    alwayslink = 1,
)

cc_library(
    name = "demo_main_hdrs",
    hdrs = ["demo_main.h"],
    visibility = ["//xrtl/port:ports_only"],
)

cc_library(
    name = "platform_demo_main",
    deps = select({
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/testing:win32_demo_main",
        ],
        "//conditions:default": [
            "//xrtl/port/common/testing:posix_demo_main",
        ],
    }),
    visibility = ["//visibility:private"],
)


# googletest header shim.
# This can be linked in to cc_library rules that require access to the gunit
# headers (such as test utility libraries/etc).
cc_library(
    name = "gtest",
    testonly = 1,
    hdrs = ["gtest.h"],
    deps = [
        "//third_party/gtest_xrtl",
    ],
)

# googletest main() entry point.
# This is linked in to cc_tests.
cc_library(
    name = "gtest_main",
    testonly = 1,
    hdrs = ["gtest.h"],
    deps = [
        ":gtest",
        "//third_party/gtest_xrtl",
    ] + select({
        "//xrtl/tools/target_config:google_internal": [
            "//xrtl/port/google/testing:google_test_main",
        ],
        "//conditions:default": [
            ":platform_gtest_main",
        ],
    }),
)

cc_library(
    name = "platform_gtest_main",
    testonly = 1,
    deps = select({
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:windows": [
            # TODO(benvanik): win32-specific test with better console output.
            "//xrtl/port/common/testing:posix_test_main",
        ],
        "//conditions:default": [
            "//xrtl/port/common/testing:posix_test_main",
        ],
    }),
    visibility = ["//visibility:private"],
)

cc_library(
    name = "file_paths_map",
    testonly = 1,
    srcs = ["file_paths_map.cc"],
    hdrs = ["file_paths_map.h"],
    deps = [
        "//xrtl/base:logging",
        "//xrtl/base:string_view",
    ],
)

cc_library(
    name = "image_loader",
    testonly = 1,
    srcs = ["image_loader.cc"],
    hdrs = ["image_loader.h"],
    deps = [
        ":file_paths_map",
        "//external:stb_image",
        "//xrtl/base:logging",
        "//xrtl/base:string_view",
    ],
)

cc_library(
    name = "image_comparison",
    testonly = 1,
    srcs = ["image_comparison.cc"],
    hdrs = ["image_comparison.h"],
    deps = [
        ":gtest",
        "//xrtl/base:logging",
    ],
)

cc_test(
    name = "image_comparison_test",
    size = "small",
    srcs = ["image_comparison_test.cc"],
    data = [
        "//xrtl/testing/testdata:test_images",
    ],
    deps = [
        ":gtest_main",
        ":image_comparison",
        ":image_loader",
    ],
)
