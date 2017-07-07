# Description:
#  Proggy fonts package.
#  http://upperbounds.net/index.php?menu=download
#  https://geoff.greer.fm/2013/12/24/programming-fonts/

package(default_visibility = ["//visibility:private"])

licenses(["notice"])  # MIT

exports_files(["LICENSE"])

genrule(
    name = "proggy_tiny_cc",
    srcs = ["ProggyTiny.ttf"],
    outs = ["proggy_tiny.cc"],
    cmd = "\n".join([
        "echo \"#include \\\"proggy_tiny.h\\\"\n\" > $@",
        "$(location @//third_party/imgui:binary_to_compressed_c) -base85 $(location ProggyTiny.ttf) proggy_tiny >> $@",
    ]),
    tools = ["@//third_party/imgui:binary_to_compressed_c"],
)

cc_library(
    name = "proggy_tiny_embedded",
    srcs = [":proggy_tiny_cc"],
    hdrs = ["proggy_tiny.h"],
    copts = [
        "-Ithird_party/proggy_fonts/",
    ],
    visibility = ["//visibility:public"],
)
