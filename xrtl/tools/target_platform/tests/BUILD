# Description:
#  Tests for our target_platform settings.
#  These can be used to test changes to the conditions backing the platform
#  selection rules.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

cc_test(
    name = "platform_test",
    srcs = select({
        "//xrtl/tools/target_platform:android": [
            "platform_android_test.cc",
        ],
        "//xrtl/tools/target_platform:emscripten": [
            "platform_emscripten_test.cc",
        ],
        "//xrtl/tools/target_platform:ios": [
            "platform_ios_test.cc",
        ],
        "//xrtl/tools/target_platform:linux": [
            "platform_linux_test.cc",
        ],
        "//xrtl/tools/target_platform:macos": [
            "platform_macos_test.cc",
        ],
        "//xrtl/tools/target_platform:windows": [
            "platform_windows_test.cc",
        ],
    }, no_match_error = "Unrecognized platform"),
    deps = [
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
)
