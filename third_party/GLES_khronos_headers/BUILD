# Description:
#  Defines targets for the standard OpenGL ES and EGL headers published by The
#  Khronos Group Inc.

package(default_visibility = ["//visibility:public"])

licenses(["permissive"])  # Custom "The Khronos Group Inc." License.

exports_files(["LICENSE"])

cc_library(
    name = "GLES",
    hdrs = [
        "include/GLES/gl.h",
        "include/GLES/glext.h",
        "include/GLES/glplatform.h",
        "include/KHR/khrplatform.h",
    ],
    includes = ["include"],
)

cc_library(
    name = "GLES2",
    hdrs = [
        "include/GLES2/gl2.h",
        "include/GLES2/gl2ext.h",
        "include/GLES2/gl2platform.h",
        "include/KHR/khrplatform.h",
    ],
    includes = ["include"],
)

cc_library(
    name = "GLES3",
    hdrs = [
        "include/GLES2/gl2ext.h",
        "include/GLES3/gl3.h",
        "include/GLES3/gl31.h",
        "include/GLES3/gl32.h",
        "include/GLES3/gl3platform.h",
        "include/KHR/khrplatform.h",
    ],
    includes = ["include"],
)

cc_library(
    name = "EGL",
    hdrs = [
        "include/EGL/egl.h",
        "include/EGL/eglext.h",
        "include/EGL/eglplatform.h",
        "include/KHR/khrplatform.h",
    ],
    includes = ["include"],
)
