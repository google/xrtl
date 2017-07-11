# Description:
#  Common port variants of //xrtl/base/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "logging_macros",
    srcs = ["logging_macros.cc"],
    hdrs = ["logging_macros.h"],
    deps = [
        "//xrtl/base:flags",
        "//xrtl/base:logging_hdrs",
        "//xrtl/base:macros",
    ] + select({
        "//xrtl/tools/target_platform:android": [
            "//xrtl/port/android/base:android_logging_sink",
        ],
        "//xrtl/tools/target_platform:emscripten": [
            "//xrtl/port/emscripten/base:emscripten_logging_sink",
        ],
        "//conditions:default": [
            ":posix_logging_sink",
        ],
    }),
)

cc_library(
    name = "logging_macros_hdrs",
    hdrs = ["logging_macros.h"],
    deps = [
        "//xrtl/base:macros",
    ],
)

cc_library(
    name = "posix_cli_main",
    srcs = ["posix_cli_main.cc"],
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
        "//xrtl/base:cli_main_hdrs",
    ],
)

cc_library(
    name = "posix_debugging",
    srcs = ["posix_debugging.cc"],
    deps = [
        "//xrtl/base:debugging_hdrs",
    ],
)

cc_library(
    name = "posix_logging_sink",
    srcs = ["posix_logging_sink.cc"],
    deps = [
        ":logging_macros_hdrs",
        "//xrtl/base:flags",
        "//xrtl/base:logging_hdrs",
        "//xrtl/base:system_clock",
    ],
)

cc_library(
    name = "posix_system_clock",
    srcs = ["posix_system_clock.cc"],
    hdrs = ["posix_system_clock.h"],
    deps = [
        "//xrtl/base:macros",
        "//xrtl/base:system_clock_hdrs",
    ],
)
