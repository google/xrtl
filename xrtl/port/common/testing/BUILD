# Description:
#  Common port variants of //xrtl/testing/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "posix_test_main",
    testonly = 1,
    srcs = ["posix_test_main.cc"],
    deps = [
        "//xrtl/base:flags",
        "//xrtl/testing:gtest",
    ],
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
)
