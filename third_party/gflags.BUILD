# Description:
#  gflags package.
#  We don't use gflags built in bazel rules as they are busted on Windows and
#  the maintainer doesn't seem to care.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

exports_files(["COPYING.txt"])

genrule(
    name = "config_h",
    srcs = ["src/config.h.in"],
    outs = ["config.h"],
    cmd = "awk '{ gsub(/^#cmakedefine/, \"//cmakedefine\"); print; }' $(<) > $(@)",
)

genrule(
    name = "gflags_declare_h",
    srcs = ["src/gflags_declare.h.in"],
    outs = ["include/gflags/gflags_declare.h"],
    cmd = ("awk '{ " +
           "gsub(/@GFLAGS_NAMESPACE@/, \"gflags\"); " +
           "gsub(/@(HAVE_STDINT_H|HAVE_SYS_TYPES_H|HAVE_INTTYPES_H|GFLAGS_INTTYPES_FORMAT_C99)@/, \"1\"); " +
           "gsub(/@([A-Z0-9_]+)@/, \"0\"); " +
           "print; }' $(<) > $(@)"),
)

genrule(
    name = "gflags_h",
    srcs = ["src/gflags.h.in"],
    outs = ["include/gflags/gflags.h"],
    cmd = ("awk '{ " +
           "gsub(/@GFLAGS_ATTRIBUTE_UNUSED@/, \"\"); " +
           "gsub(/@INCLUDE_GFLAGS_NS_H@/, \"\"); " +
           "print; }' $(<) > $(@)"),
)

genrule(
    name = "gflags_completions_h",
    srcs = ["src/gflags_completions.h.in"],
    outs = ["include/gflags/gflags_completions.h"],
    cmd = "awk '{ gsub(/@GFLAGS_NAMESPACE@/, \"gflags\"); print; }' $(<) > $(@)",
)

native.cc_library(
    name = "gflags",
    hdrs = [
        ":gflags_completions_h",
        ":gflags_declare_h",
        ":gflags_h",
    ],
    srcs = [
        ":config_h",
        "src/gflags.cc",
        "src/gflags_completions.cc",
        "src/gflags_reporting.cc",
        "src/mutex.h",
        "src/util.h",
    ] + select({
        "@//xrtl/tools/target_platform:windows": [
            "src/windows_port.cc",
            "src/windows_port.h",
        ],
        "//conditions:default": [],
    }),
    includes = ["include/"],
    copts = [
        "-DHAVE_STDINT_H",
        "-DHAVE_SYS_TYPES_H",
        "-DHAVE_INTTYPES_H",
        "-DHAVE_SYS_STAT_H",
        "-DHAVE_UNISTD_H",
        "-DHAVE_STRTOLL",
        "-DHAVE_STRTOQ",
        "-DHAVE_RWLOCK",
        "-DGFLAGS_INTTYPES_FORMAT_C99",
        "-DNO_THREADS",
    ] + select({
        "@//xrtl/tools/target_platform:windows": [
            "-DOS_WINDOWS",
            "-DHAVE_SHLWAPI_H",
        ],
        "//conditions:default": [
            "-DHAVE_FNMATCH_H",
            "-DHAVE_PTHREAD",
        ],
    }),
    defines = [
        "GFLAGS_IS_A_DLL=0",
        "GFLAGS_DLL_DEFINE_FLAG=",
    ],
    linkopts = select({
        "@//xrtl/tools/target_platform:android": [],
        "@//xrtl/tools/target_platform:windows": [
            "-Wl,shlwapi.lib",
        ],
        "//conditions:default": [
            "-lpthread",
        ],
    }),
    visibility = ["//visibility:public"],
)
