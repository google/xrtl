# Description:
#  Testdata for //xrtl/base.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

proto_library(
    name = "proto_util_test_proto",
    srcs = ["proto_util_test.proto"],
)

cc_proto_library(
    name = "proto_util_test_cc_proto",
    deps = [":proto_util_test_proto"],
)
