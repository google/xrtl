# Description:
#  config_settings for our various OS and CPU targets.
#  These come in variants for OS level and OS + CPU.
#  Assume that //conditions:default is linux (or something like it) and not the
#  current host OS.
#
#  Most often you should switch on one of the top-level OS config settings. Only
#  switch on CPU-specific settings when required.
#
#  Some platforms contain synthetic rules (such as :ios_simulator) that can be
#  used to target any architecture in a specific environment.
#
#  Example:
#  cc_library(
#      name = "my_lib",
#      srcs = select({
#          "//xrtl/tools/target_platform:android": ["some_android.cc"],
#          "//xrtl/tools/target_platform:ios": ["some_ios.cc"],
#          "//conditions:default": ["some_generic.cc"],
#      }),
#  )

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

load(
    ":conditions.bzl",
    "CONDITION_ANDROID",
    "CONDITION_ANDROID_ARM_V7A",
    "CONDITION_ANDROID_ARM_V8A",
    "CONDITION_ANDROID_X86_32",
    "CONDITION_ANDROID_X86_64",
    "CONDITION_EMSCRIPTEN",
    "CONDITION_EMSCRIPTEN_ASMJS",
    "CONDITION_EMSCRIPTEN_WASM_32",
    "CONDITION_EMSCRIPTEN_WASM_64",
    "CONDITION_IOS",
    "CONDITION_IOS_ARM_V7A",
    "CONDITION_IOS_ARM_V8A",
    "CONDITION_IOS_X86_32",
    "CONDITION_IOS_X86_64",
    "CONDITION_LINUX",
    "CONDITION_LINUX_X86_64",
    "CONDITION_MACOS",
    "CONDITION_MACOS_X86_64",
    "CONDITION_WINDOWS",
    "CONDITION_WINDOWS_X86_64",
)

# Header that defines platform and architecture preprocessor values.
# These can be used to conditionally enable code paths or dependencies.
cc_library(
    name = "target_platform",
    srcs = ["target_platform.cc"],
    hdrs = ["target_platform.h"],
    defines = select(
        {
            # NOTE: one platform must be defined.
            # If you are getting bazel select errors here add the platform to
            # the list.
            "//xrtl/tools/target_platform:android": [],
            "//xrtl/tools/target_platform:emscripten": [],
            "//xrtl/tools/target_platform:ios": [],
            "//xrtl/tools/target_platform:linux": [],
            "//xrtl/tools/target_platform:macos": [],
            "//xrtl/tools/target_platform:windows": [],

            # Special case for tulsi, which processes files without a config.
            # TODO(benvanik): figure out a way to avoid this so we get good
            #                 errors on Unrecognized platforms.
            "//conditions:default": [],
        },
        no_match_error = "Unrecognized platform",
    ),
)

#===============================================================================
# Android
#===============================================================================

# Any Android device (physical or emulator).
config_setting(
    name = "android",
    values = CONDITION_ANDROID,
)

config_setting(
    name = "android_emulator",
    values = {
        "define": "XRTL_PLATFORM_ANDROID_EMULATOR=1",
    },
)

config_setting(
    name = "android_arm_v7a",
    values = CONDITION_ANDROID_ARM_V7A,
)

config_setting(
    name = "android_arm_v8a",
    values = CONDITION_ANDROID_ARM_V8A,
)

config_setting(
    name = "android_x86_32",
    values = CONDITION_ANDROID_X86_32,
)

config_setting(
    name = "android_x86_64",
    values = CONDITION_ANDROID_X86_64,
)

#===============================================================================
# Emscripten
#===============================================================================

# Emscripten for asmjs or wasm.
config_setting(
    name = "emscripten",
    values = CONDITION_EMSCRIPTEN,
)

config_setting(
    name = "emscripten_asmjs",
    values = {
        "crosstool_top": "TODO",
        "cpu": "asmjs",
    },
)

config_setting(
    name = "emscripten_wasm_32",
    values = CONDITION_EMSCRIPTEN_WASM_32,
)

config_setting(
    name = "emscripten_wasm_64",
    values = CONDITION_EMSCRIPTEN_WASM_64,
)

#===============================================================================
# iOS
#===============================================================================

# Any iOS device (physical or simulator).
config_setting(
    name = "ios",
    values = CONDITION_IOS,
)

config_setting(
    name = "ios_simulator",
    values = {
        "define": "XRTL_PLATFORM_IOS_SIMULATOR=1",
    },
)

config_setting(
    name = "ios_arm_v7a",
    values = CONDITION_IOS_ARM_V7A,
)

config_setting(
    name = "ios_arm_v8a",
    values = CONDITION_IOS_ARM_V8A,
)

config_setting(
    name = "ios_x86_32",
    values = CONDITION_IOS_X86_32,
)

config_setting(
    name = "ios_x86_64",
    values = CONDITION_IOS_X86_64,
)

#===============================================================================
# Linux
#===============================================================================

# Linux, which is usually expressed as //conditions:default in selects.
config_setting(
    name = "linux",
    values = CONDITION_LINUX,
)

config_setting(
    name = "linux_x86_64",
    values = CONDITION_LINUX_X86_64,
)

#===============================================================================
# MacOS
#===============================================================================

# MacOS.
config_setting(
    name = "macos",
    values = CONDITION_MACOS,
)

config_setting(
    name = "macos_x86_64",
    values = CONDITION_MACOS_X86_64,
)

#===============================================================================
# Windows
#===============================================================================

# Windows (desktop).
config_setting(
    name = "windows",
    values = CONDITION_WINDOWS,
)

config_setting(
    name = "windows_x86_64",
    values = CONDITION_WINDOWS_X86_64,
)
