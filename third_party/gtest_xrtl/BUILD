# Description:
#  gtest bazel BUILD file.
#  We need to do this separate from //third_party/gtest/ (or a gtest.BUILD)
#  as we need to reference //xrtl for our flags/logging code.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])

genrule(
    name = "gtest_port_hdr_gen",
    outs = ["gtest/googletest/gtest/internal/custom/gtest-port.h"],
    cmd = "\n".join([
        "cat <<'EOF' > $@",
        "#ifndef GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PORT_H_",
        "#define GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PORT_H_",
        "#include \"gtest/internal/gtest-port-arch.h\"",
        "#include \"xrtl/base/logging.h\"",
        "#include \"xrtl/base/flags.h\"",
        "",
        "#define GTEST_TEST_FILTER_ENV_VAR_ \"TESTBRIDGE_TEST_ONLY\"",
        "",
        "#define GTEST_USE_OWN_FLAGFILE_FLAG_ 0",
        "#define GTEST_FLAG_SAVER_ gflags::FlagSaver",
        "#define GTEST_FLAG(name) FLAGS_gunit_##name",
        "#define GTEST_DECLARE_bool_(name) DECLARE_bool(gunit_##name)",
        "#define GTEST_DECLARE_int32_(name) DECLARE_int32(gunit_##name)",
        "#define GTEST_DECLARE_string_(name) DECLARE_string(gunit_##name)",
        "#define GTEST_DEFINE_bool_(name, default_val, doc) DEFINE_bool(gunit_##name, default_val, doc)",
        "#define GTEST_DEFINE_int32_(name, default_val, doc) DEFINE_int32(gunit_##name, default_val, doc)",
        "#define GTEST_DEFINE_string_(name, default_val, doc) DEFINE_string(gunit_##name, default_val, doc)",
        "",
        # TODO(benvanik): this causes dyld issues on osx; figure out why
        #"#define GTEST_LOG_(severity) LOG(severity)",
        "#define GTEST_LOG_(severity) ::testing::internal::GTestLog(::testing::internal::GTEST_##severity, __FILE__, __LINE__).GetStream()",
        "namespace testing {",
        "namespace internal {",
        "inline void LogToStderr() {}",
        "inline void FlushInfoLog() { xrtl::FlushLog(); }",
        "}  // namespace internal",
        "}  // namespace testing",
        #"#define GTEST_CHECK_ CHECK",
        "",
        "#endif  // GTEST_INCLUDE_GTEST_INTERNAL_CUSTOM_GTEST_PORT_H_",
        "EOF",
    ]),
)

cc_library(
    name = "gtest_xrtl",
    srcs = [
        "gtest/googlemock/src/gmock-all.cc",
        "gtest/googletest/src/gtest-all.cc",
    ],
    hdrs = [
        ":gtest_port_hdr_gen",
    ] + glob([
        "gtest/**/*.h",
        "gtest/googletest/src/*.cc",
        "gtest/googlemock/src/*.cc",
    ]),
    deps = [
        "//xrtl/base:flags",
        "//xrtl/base:logging",
    ],
    includes = [
        "gtest/googlemock",
        "gtest/googlemock/include",
        "gtest/googletest",
        "gtest/googletest/include",
    ],
    visibility = ["//visibility:public"],
)
