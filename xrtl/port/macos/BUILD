# Description:
#  MacOS port root. Packages under here match those in the root XRTL package.
#  May use some of the common port files under //xrtl/port/common/ and
#  //xrtl/port/apple/.
#
#  Targets:
#  - Xcode Version: 8.2
#  - MacOS SDK: 10?
#  - MacOS Version: 10?

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "package_platforms")

package_platforms(default_platforms = ["//xrtl/tools/target_platform:macos"])

#

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
