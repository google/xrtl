"""Platform constants."""

# Friendly platform names.
PLATFORM_NAMES = [
    "android",
    "emscripten",
    "ios",
    "linux",
    "macos",
    "windows",
]

# Fully-qualified platform name. These can be used in select()s.
# Example: //xrtl/tools/target_platform:ios
PLATFORMS = ["//xrtl/tools/target_platform:%s" % (platform_name) for platform_name in PLATFORM_NAMES]
