# Description:
#  Common graphics utilities for backend implementations.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "memory_command_buffer",
    srcs = ["memory_command_buffer.cc"],
    hdrs = ["memory_command_buffer.h"],
    deps = [
        ":memory_command_buffer_reader",
        ":memory_command_buffer_writer",
        ":memory_command_encoder",
        "//xrtl/base:arena",
        "//xrtl/gfx:command_buffer",
    ],
)

cc_test(
    name = "memory_command_buffer_test",
    size = "small",
    srcs = ["memory_command_buffer_test.cc"],
    deps = [
        ":memory_command_buffer",
        ":memory_command_decoder",
        "//xrtl/gfx/testing:partial_command_encoder",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "memory_command_buffer_reader",
    srcs = ["memory_command_buffer_reader.cc"],
    hdrs = ["memory_command_buffer_reader.h"],
    deps = [
        ":memory_commands",
        "//xrtl/base:array_view",
        "//xrtl/base:logging",
        "//xrtl/base:ref_ptr",
        "//xrtl/gfx:command_encoder",
    ],
)

cc_library(
    name = "memory_command_buffer_writer",
    srcs = ["memory_command_buffer_writer.cc"],
    hdrs = ["memory_command_buffer_writer.h"],
    deps = [
        ":memory_commands",
        "//xrtl/base:arena",
        "//xrtl/base:array_view",
        "//xrtl/base:logging",
        "//xrtl/base:ref_ptr",
        "//xrtl/gfx:command_encoder",
    ],
)

cc_test(
    name = "memory_command_buffer_reader_writer_test",
    size = "small",
    srcs = ["memory_command_buffer_reader_writer_test.cc"],
    deps = [
        ":memory_command_buffer_reader",
        ":memory_command_buffer_writer",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "memory_command_decoder",
    srcs = ["memory_command_decoder.cc"],
    hdrs = ["memory_command_decoder.h"],
    deps = [
        ":memory_command_buffer_reader",
        ":memory_commands",
        "//xrtl/base:logging",
        "//xrtl/gfx:command_buffer",
        "//xrtl/gfx:command_encoder",
    ],
)

cc_library(
    name = "memory_command_encoder",
    srcs = ["memory_command_encoder.cc"],
    hdrs = ["memory_command_encoder.h"],
    deps = [
        ":memory_command_buffer_writer",
        ":memory_commands",
        "//xrtl/gfx:command_buffer",
        "//xrtl/gfx:command_encoder",
    ],
)

cc_library(
    name = "memory_commands",
    hdrs = ["memory_commands.h"],
    deps = [
        "//xrtl/gfx:command_encoder",
        "//xrtl/gfx:framebuffer",
    ],
)
