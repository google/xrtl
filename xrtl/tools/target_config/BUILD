# Description:
#  config_settings for compilation modes and release configurations.
#  These generally map to the bzel --compilation_mode (-c) setting, but also
#  also define variants for special testing, benchmarking, and experimental
#  builds.
#
#  Example:
#  cc_library(
#    name = "my_lib",
#    deps = select({
#        "//xrtl/tools/target_config:dbg": [":debug_settings"],
#        "//xrtl/tools/target_config:opt": [":release_settings"],
#        "//conditions:default": [":default_settings"],
#    }),
#  )

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

# Header that defines config preprocessor values.
# These can be used to conditionally enable code paths or dependencies.
cc_library(
    name = "target_config",
    srcs = ["target_config.cc"],
    hdrs = ["target_config.h"],
    defines = select({
        "//xrtl/tools/target_config:dbg": [
            "XRTL_CONFIG_DBG=1",
        ],
        "//xrtl/tools/target_config:fastbuild": [
            "XRTL_CONFIG_FASTBUILD=1",
        ],
        "//xrtl/tools/target_config:opt": [
            "XRTL_CONFIG_OPT=1",
        ],
    }) + select({
        "//xrtl/tools/target_config:google_internal": [
            "XRTL_CONFIG_GOOGLE_INTERNAL=1",
        ],
        "//conditions:default": [],
    }) + select({
        "//xrtl/tools/target_config:minimal_logging": [],
        "//conditions:default": [
            "XRTL_CONFIG_LOGGING_VERBOSE=1",
        ],
    }) + select({
        "//xrtl/tools/target_config:swiftshader": [
            "XRTL_CONFIG_SWIFTSHADER=1",
        ],
        "//conditions:default": [],
    }),
)

# Debug; all runtime checks, verbose logging, and debug symbols.
#
# $ bazel build -c dbg ...
config_setting(
    name = "dbg",
    values = {"compilation_mode": "dbg"},
)

# Fast build; some runtime checks, detailed logging, and debug symbols.
# This is the default and will likely be what you want to use unless profiling.
#
# $ bazel build -c fastbuild ...
config_setting(
    name = "fastbuild",
    values = {"compilation_mode": "fastbuild"},
)

# Optimized; no runtime checks and terse logging.
#
# $ bazel build -c opt ...
config_setting(
    name = "opt",
    values = {"compilation_mode": "opt"},
)

# Enable WTF tracing.
# This will compile in all WTF tracers and have applications dump trace files.
# It does have an overhead but is generally worth keeping enabled in fastbuild
# and optionally enabling in opt when required. Do not run benchmarks with it
# enabled, though, as it'll skew timing.
#
# $ bazel build --define=WTF_ENABLE=1 ...
config_setting(
    name = "tracing_enabled",
    values = {
        "define": "WTF_ENABLE=1",
    },
)

# Compile for as in the internal google repository.
config_setting(
    name = "google_internal",
    values = {
        "xcode_version_config": "//tools/osx:fom_xcodes",
    },
)

# Reducing logging at compile time.
config_setting(
    name = "minimal_logging",
    values = {
        "define": "MINIMAL_LOGGING=1",
    },
)

# Use Swiftshader for GLES/EGL instead of system libraries.
#
# $ bazel build --define=SWIFTSHADER=1 ...
config_setting(
    name = "swiftshader",
    values = {
        "define": "SWIFTSHADER=1",
    },
)
