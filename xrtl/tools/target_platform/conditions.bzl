# Description:
#  Condition values for config_settings in target_platform.
#  These allow us to use a bit of skylark magic more easily
#  and swap out this file for other versions.

CONDITION_ANDROID = {
    "crosstool_top": "TODO",
}

CONDITION_ANDROID_ARM_V7A = {
    "crosstool_top": "TODO",
    "cpu": "armeabi-v7a",
}

CONDITION_ANDROID_ARM_V8A = {
    "crosstool_top": "TODO",
    "cpu": "arm64-v8a",
}

CONDITION_ANDROID_X86_32 = {
    "crosstool_top": "TODO",
    "cpu": "x86",
}

CONDITION_ANDROID_X86_64 = {
    "crosstool_top": "TODO",
    "cpu": "x86_64",
}

CONDITION_EMSCRIPTEN = {
    "crosstool_top": "TODO",
}

CONDITION_EMSCRIPTEN_ASMJS = {
    "crosstool_top": "TODO",
    "cpu": "asmjs",
}

CONDITION_EMSCRIPTEN_WASM_32 = {
    "crosstool_top": "TODO",
    "cpu": "asmjs",
}

CONDITION_EMSCRIPTEN_WASM_64 = {
    "crosstool_top": "TODO",
    "cpu": "asmjs",
}

CONDITION_IOS = {
    "crosstool_top": "TODO",
}

CONDITION_IOS_ARM_V7A = {
    "cpu": "ios_armv7",
}

CONDITION_IOS_ARM_V8A = {
    "cpu": "ios_arm64",
}

CONDITION_IOS_X86_32 = {
    "cpu": "ios_i386",
}

CONDITION_IOS_X86_64 = {
    "cpu": "ios_x86_64",
}

CONDITION_LINUX = {
    "cpu": "k8",
}

CONDITION_LINUX_X86_64 = {
    "cpu": "k8",
}

CONDITION_MACOS = {
    "cpu": "darwin",
}

CONDITION_MACOS_X86_64 = {
    "cpu": "darwin",
}

CONDITION_WINDOWS = {
    "cpu": "x64_windows_msvc",
}

CONDITION_WINDOWS_X86_64 = {
    "cpu": "x64_windows_msvc",
}
