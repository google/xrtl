# Description:
#  imgui package.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])  # MIT

cc_library(
    name = "imgui",
    srcs = [
        "imconfig.cpp",
        "imconfig.h",
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_internal.h",
    ],
    hdrs = [
        "imgui.h",
    ],
    copts = [
        "-DIMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION",
        "-DIMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION",
    ],
    defines = [
        "IMGUI_INCLUDE_IMCONFIG_H",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@//xrtl/base:logging",
        "@com_github_gtruc_glm//:glm",

        # imgui requires an internal textedit implementation.
        "@com_github_nothings_stb//:stb_textedit_hdrs",
        "@com_github_nothings_stb//:stb_rect_pack",
        "@com_github_nothings_stb//:stb_truetype",
    ],
)

cc_binary(
    name = "binary_to_compressed_c",
    srcs = ["extra_fonts/binary_to_compressed_c.cpp"],
    visibility = ["//visibility:public"],
)
