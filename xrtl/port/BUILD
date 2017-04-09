# Description:
#  Platform-specific implementations and features.
#  Anything that only compiles for a specific platform should be kept under the
#  appropriate directory here, with implementations shared amongst several
#  platforms under common/.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

# The following package groups can be used to restrict code that cannot be
# depended on outside of the respective ports path such as system headers
# and libraries.

package_group(
    name = "ports_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/...",
    ],
)

package_group(
    name = "android_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/android/...",
    ],
)

package_group(
    name = "emscripten_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/emscripten/...",
    ],
)

package_group(
    name = "ios_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/ios/...",
    ],
)

package_group(
    name = "linux_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/linux/...",
    ],
)

package_group(
    name = "macos_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/macos/...",
    ],
)

package_group(
    name = "windows_only",
    packages = [
        "//third_party/...",
        "//xrtl/port/windows/...",
    ],
)
