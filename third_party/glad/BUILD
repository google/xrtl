# Description:
#  Generated OpenGL loader files from the GLAD web service.
#  https://github.com/Dav1dde/glad
#  http://glad.dav1d.de/

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT

exports_files(["LICENSE"])

COMMON_COPTS = select({
    "@//xrtl/tools/target_platform:windows": [
        "/TP",  # Compile as C++.
    ],
    "//conditions:default": [
        "-x", "c++",
    ],
})

cc_library(
    name = "glad",
    srcs = ["src/glad.c"],
    hdrs = ["include/glad/glad.h"],
    deps = [
        "//third_party/GLES_khronos_headers:GLES3",
    ],
    copts = COMMON_COPTS,
    includes = ["include"],
)

cc_library(
    name = "glad_egl",
    srcs = ["src/glad_egl.c"],
    hdrs = ["include/glad/glad_egl.h"],
    deps = [
        ":glad",
        "//third_party/GLES_khronos_headers:EGL",
    ],
    copts = COMMON_COPTS,
    includes = ["include"],
)
