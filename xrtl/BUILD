# Description:
#  Cross-platform Real-time Rendering Library.
#  https://github.com/google/xrtl

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

# Restricts most packages to use by //xrtl/... only.
# Selective rules we wish to support to external users are set to
# //visibility:public.
package_group(
    name = "internal",
    packages = [
        "//third_party/...",
        "//xrtl/...",
    ],
)
