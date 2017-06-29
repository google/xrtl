# Description:
#  stb package.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

cc_library(
    name = "stb_image",
    hdrs = [
        "stb_image.h",
        "stb_image_write.h",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)

# TODO(scotttodd): genrule a cc or header that uses ifndef with
#     STB_IMAGE_IMPLEMENTATION
