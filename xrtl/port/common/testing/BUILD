# Description:
#  Common port variants of //xrtl/testing/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "posix_demo_main",
    srcs = ["posix_demo_main.cc"],
    linkopts = select({
        # TODO(benvanik): find a way to remove this requirement.
        "//xrtl/tools/target_platform:ios": [
            "-framework CoreFoundation",
        ],
        "//xrtl/tools/target_platform:macos": [
            "-framework CoreFoundation",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//xrtl/testing:demo_main_hdrs",
    ],
)

cc_library(
    name = "posix_test_main",
    testonly = 1,
    srcs = ["posix_test_main.cc"],
    linkopts = select({
        # TODO(benvanik): find a way to remove this requirement.
        "//xrtl/tools/target_platform:ios": [
            "-framework CoreFoundation",
        ],
        "//xrtl/tools/target_platform:macos": [
            "-framework CoreFoundation",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//xrtl/base:flags",
        "//xrtl/base:logging",
        "//xrtl/testing:file_manifest",
        "//xrtl/testing:gtest",
    ],
)
