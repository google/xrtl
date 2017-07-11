# Description:
#  Shaders used by xrtl::ui.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

genrule(
    name = "imgui_overlay_frag_files",
    srcs = ["imgui_overlay.frag"],
    outs = [
        "imgui_overlay_frag.cc",
        "imgui_overlay_frag.h",
    ],
    cmd = " ".join([
        "$(location //xrtl/gfx/spirv:shader_tool)",
        "$(SRCS)",
        "--output_namespace=xrtl::ui::shaders",
        "--output_variable=kImGuiOverlayFragSpirV",
        "--output_spirv_h=$(location imgui_overlay_frag.h)",
        "--output_spirv_cc=$(location imgui_overlay_frag.cc)",
    ]),
    tools = ["//xrtl/gfx/spirv:shader_tool"],
)

cc_library(
    name = "imgui_overlay_frag",
    srcs = [":imgui_overlay_frag.cc"],
    hdrs = [":imgui_overlay_frag.h"],
)

genrule(
    name = "imgui_overlay_vert_files",
    srcs = ["imgui_overlay.vert"],
    outs = [
        "imgui_overlay_vert.cc",
        "imgui_overlay_vert.h",
    ],
    cmd = " ".join([
        "$(location //xrtl/gfx/spirv:shader_tool)",
        "$(SRCS)",
        "--output_namespace=xrtl::ui::shaders",
        "--output_variable=kImGuiOverlayVertSpirV",
        "--output_spirv_h=$(location imgui_overlay_vert.h)",
        "--output_spirv_cc=$(location imgui_overlay_vert.cc)",
    ]),
    tools = ["//xrtl/gfx/spirv:shader_tool"],
)

cc_library(
    name = "imgui_overlay_vert",
    srcs = [":imgui_overlay_vert.cc"],
    hdrs = [":imgui_overlay_vert.h"],
)
