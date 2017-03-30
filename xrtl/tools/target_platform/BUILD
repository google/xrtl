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

# Header that defines platform and architecture preprocessor values.
# These can be used to conditionally enable code paths or dependencies.
cc_library(
    name = "target_platform",
    srcs = ["target_platform.cc"],
    hdrs = ["target_platform.h"],
    defines = select({
        # NOTE: one platform must be defined.
        # If you are getting bazel select errors here add the platform to
        # the list.
        "//xrtl/tools/target_platform:android": [],
        "//xrtl/tools/target_platform:emscripten": [],
        "//xrtl/tools/target_platform:ios": [],
        "//xrtl/tools/target_platform:linux": [],
        "//xrtl/tools/target_platform:macos": [],
        "//xrtl/tools/target_platform:windows": [],
    }),
)

#===============================================================================
# Operating Systems
#===============================================================================

# Any Android device (physical or emulator).
config_setting(
    name = "android",
    values = {
        "crosstool_top": "TODO",
    },
)

# Emscripten for asmjs or wasm.
config_setting(
    name = "emscripten",
    values = {
        "crosstool_top": "TODO",
    },
)

# Any iOS device (physical or simulator).
config_setting(
    name = "ios",
    values = {
        "crosstool_top": "TODO",
    },
)

# Linux, which is usually expressed as //conditions:default in selects.
config_setting(
    name = "linux",
    values = {
        "cpu": "k8",
    },
)

# MacOS.
config_setting(
    name = "macos",
    values = {
        "cpu": "darwin",
    },
)

# Windows (desktop).
config_setting(
    name = "windows",
    values = {
        "crosstool_top": "TODO",
    },
)

#===============================================================================
# Android
#===============================================================================

config_setting(
    name = "android_emulator",
    values = {
        "define": "XRTL_PLATFORM_ANDROID_EMULATOR=1",
    },
)

config_setting(
    name = "android_arm",
    values = {
        "crosstool_top": "TODO",
        "cpu": "armeabi-v7a",
    },
)

config_setting(
    name = "android_arm64",
    values = {
        "crosstool_top": "TODO",
        "cpu": "arm64-v8a",
    },
)

config_setting(
    name = "android_x86",
    values = {
        "crosstool_top": "TODO",
        "cpu": "x86",
    },
)

config_setting(
    name = "android_x86_64",
    values = {
        "crosstool_top": "TODO",
        "cpu": "x86_64",
    },
)

#===============================================================================
# Emscripten
#===============================================================================

config_setting(
    name = "emscripten_asmjs",
    values = {
        "crosstool_top": "TODO",
        "cpu": "asmjs",
    },
)

config_setting(
    name = "emscripten_wasm",
    values = {
        "crosstool_top": "TODO",
        "cpu": "asmjs",
    },
)

#===============================================================================
# iOS
#===============================================================================

config_setting(
    name = "ios_simulator",
    values = {
        "define": "XRTL_PLATFORM_IOS_SIMULATOR=1",
    },
)

config_setting(
    name = "ios_arm",
    values = {
        "ios_cpu": "armv7",
    },
)

config_setting(
    name = "ios_arm64",
    values = {
        "ios_cpu": "arm64",
    },
)

config_setting(
    name = "ios_x86",
    values = {
        "ios_cpu": "i386",
    },
)

config_setting(
    name = "ios_x86_64",
    values = {
        "ios_cpu": "x86_64",
    },
)

#===============================================================================
# Linux
#===============================================================================

config_setting(
    name = "linux_x86_64",
    values = {
        "cpu": "k8",
    },
)

#===============================================================================
# MacOS
#===============================================================================

config_setting(
    name = "macos_x86_64",
    values = {
        "cpu": "darwin",
    },
)

#===============================================================================
# Windows
#===============================================================================

config_setting(
    name = "windows_x86_64",
    values = {
        "cpu": "x64_windows_msvc",
    },
)
