# Description:
#  stb package.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

cc_library(
    name = "stb_image",
    srcs = [":stb_image_impl"],
    hdrs = ["stb_image.h"],
    copts = select({
        "@//xrtl/tools/target_platform:windows": [],
        "//conditions:default": [
            "-Wno-unused-function",
        ],
    }),
    defines = [
        "STBI_NO_LINEAR",  # no loadf
        "STBI_ONLY_PNG",  # only .png support
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)

genrule(
    name = "stb_image_impl",
    outs = ["stb_image.cc"],
    cmd = "\n".join([
        "cat <<'EOF' > $@",
        "#define STB_IMAGE_IMPLEMENTATION",
        "#define STB_IMAGE_WRITE_IMPLEMENTATION",
        "#include \"stb_image.h\"",
        "EOF",
    ]),
)

cc_library(
    name = "stb_image_write",
    srcs = [":stb_image_write_impl"],
    hdrs = ["stb_image_write.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

genrule(
    name = "stb_image_write_impl",
    outs = ["stb_image_write.cc"],
    cmd = "\n".join([
        "cat <<'EOF' > $@",
        "#define STB_IMAGE_WRITE_IMPLEMENTATION",
        "#include \"stb_image_write.h\"",
        "EOF",
    ]),
)

cc_library(
    name = "stb_rect_pack",
    srcs = [":stb_rect_pack_impl"],
    hdrs = ["stb_rect_pack.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

genrule(
    name = "stb_rect_pack_impl",
    outs = ["stb_rect_pack.cc"],
    cmd = "\n".join([
        "cat <<'EOF' > $@",
        "#define STB_RECT_PACK_IMPLEMENTATION",
        "#include \"stb_rect_pack.h\"",
        "EOF",
    ]),
)

cc_library(
    name = "stb_textedit",
    srcs = [":stb_textedit_impl"],
    hdrs = ["stb_textedit.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = [":stb_textedit_hdrs"],
)

cc_library(
    name = "stb_textedit_hdrs",
    hdrs = ["stb_textedit.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

genrule(
    name = "stb_textedit_impl",
    outs = ["stb_textedit.cc"],
    cmd = "\n".join([
        "cat <<'EOF' > $@",
        "#define STB_TRUETYPE_IMPLEMENTATION",
        "#include \"stb_textedit.h\"",
        "EOF",
    ]),
)

cc_library(
    name = "stb_truetype",
    srcs = [":stb_truetype_impl"],
    hdrs = ["stb_truetype.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = [
        ":stb_rect_pack",
    ],
)

genrule(
    name = "stb_truetype_impl",
    outs = ["stb_truetype.cc"],
    cmd = "\n".join([
        "cat <<'EOF' > $@",
        "#include \"stb_rect_pack.h\"",
        "#define STB_TRUETYPE_IMPLEMENTATION",
        "#include \"stb_truetype.h\"",
        "EOF",
    ]),
)
