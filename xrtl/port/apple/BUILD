# Description:
#  Code common to Apple platforms (iOS, MacOS, etc).

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = [
    "//xrtl/tools/target_platform:ios",
    "//xrtl/tools/target_platform:macos",
])

#

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
