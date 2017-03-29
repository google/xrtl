# Description:
#  Common port root. Packages under here match those in the root XRTL package.
#  The code in these packages may be shared by one or more of the platform ports
#  either directly or as a base implementation. Not all code may be used by
#  all platforms.
#
#  We keep this common code here instead of outside of port/ so that we can
#  ensure all non-port/ code is platform agnostic. Were we to put these files
#  directly under //xrtl/... it becomes very easy to inadvertently pull in
#  platform-family-specific code. By keeping the code separate we also allow
#  bazel recursive package actions to work without any particular --config
#  specified.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0
