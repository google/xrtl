# Description:
#  cc_toolchain for --config=msan.
#  This is required for msan as we need to use our own locally-built libc++.
#  See //tools/ci/jenkins/linux/setup_node.sh for the libc++ setup instructions.

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "empty",
    srcs = [],
)

cc_toolchain_suite(
    name = "msan",
    toolchains = {
        "k8|compiler": ":cc-compiler-k8-msan",
    },
)

cc_toolchain(
    name = "cc-compiler-k8-msan",
    all_files = ":empty",
    compiler_files = ":empty",
    cpu = "local",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":empty",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 1,
)
