# Description:
#  Threading library.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

load("//xrtl/testing:portable_testing.bzl", "portable_cc_test")

cc_library(
    name = "threading",
    hdrs = [
        "event.h",
        "semaphore.h",
        "thread.h",
        "timer.h",
        "wait_handle.h",
    ],
    deps = [
        ":event",
        ":semaphore",
        ":thread",
        ":timer",
        ":wait_handle",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "threading_hdrs",
    hdrs = [
        "event.h",
        "message_loop.h",
        "semaphore.h",
        "thread.h",
        "timer.h",
        "wait_handle.h",
    ],
    visibility = ["//xrtl/port:ports_only"],
    deps = [
        "//xrtl/base:intrusive_list",
        "//xrtl/base:intrusive_pool",
        "//xrtl/base:logging",
        "//xrtl/base:macros",
        "//xrtl/base:ref_ptr",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "event",
    hdrs = ["event.h"],
    deps = [
        ":wait_handle",
    ] + select({
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/base/threading:win32_event",
        ],
        "//conditions:default": [
            "//xrtl/port/common/base/threading:pthreads_event",
        ],
    }),
)

portable_cc_test(
    name = "event_test",
    size = "small",
    srcs = ["event_test.cc"],
    deps = [
        ":event",
        ":thread",
        "//xrtl/base:stopwatch",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "message_loop",
    srcs = ["message_loop.cc"],
    hdrs = ["message_loop.h"],
    deps = [
        ":thread",
        ":wait_handle",
        "//xrtl/base:intrusive_list",
        "//xrtl/base:intrusive_pool",
        "//xrtl/base:logging",
        "//xrtl/base:ref_ptr",
    ] + select({
        "//xrtl/tools/target_platform:ios": [
            "//xrtl/port/apple/base/threading:cfrunloop_message_loop",
        ],
        "//xrtl/tools/target_platform:macos": [
            "//xrtl/port/apple/base/threading:cfrunloop_message_loop",
        ],
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/base/threading:win32_message_loop",
        ],
        "//conditions:default": [
            "//xrtl/port/common/base/threading:epoll_message_loop",
        ],
    }),
)

portable_cc_test(
    name = "message_loop_test",
    size = "small",
    srcs = ["message_loop_test.cc"],
    deps = [
        ":event",
        ":message_loop",
        ":thread",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "semaphore",
    hdrs = ["semaphore.h"],
    deps = [
        ":wait_handle",
    ] + select({
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/base/threading:win32_semaphore",
        ],
        "//conditions:default": [
            "//xrtl/port/common/base/threading:pthreads_semaphore",
        ],
    }),
)

portable_cc_test(
    name = "semaphore_test",
    size = "small",
    srcs = ["semaphore_test.cc"],
    deps = [
        ":event",
        ":semaphore",
        ":thread",
        "//xrtl/base:stopwatch",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "thread",
    hdrs = ["thread.h"],
    deps = [
        ":thread_base",
        ":wait_handle",
        "@com_google_absl//absl/types:span",
        "//xrtl/base:macros",
    ] + select({
        "//xrtl/tools/target_platform:windows": [
            "//xrtl/port/windows/base/threading:win32_thread",
        ],
        "//conditions:default": [
            "//xrtl/port/common/base/threading:pthreads_thread",
        ],
    }),
)

cc_library(
    name = "thread_base",
    srcs = ["thread.cc"],
    hdrs = ["thread.h"],
    visibility = ["//xrtl/port:ports_only"],
    deps = [
        ":wait_handle",
        "//xrtl/base:macros",
        "@com_google_absl//absl/types:span",
    ],
)

portable_cc_test(
    name = "thread_test",
    size = "small",
    srcs = ["thread_test.cc"],
    deps = [
        ":event",
        ":semaphore",
        ":thread",
        "//xrtl/base:stopwatch",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "timer",
    hdrs = ["timer.h"],
    deps = [
        ":wait_handle",
    ] + select({
        "//xrtl/tools/target_platform:windows": [
            # TODO(benvanik): implement win32 timer.
            # "//xrtl/port/windows/base/threading:win32_timer",
        ],
        "//conditions:default": [
            # TODO(benvanik): implement pthreads timer.
            # "//xrtl/port/common/base/threading:pthreads_timer",
        ],
    }),
)

portable_cc_test(
    name = "timer_test",
    size = "small",
    srcs = ["timer_test.cc"],
    deps = [
        ":thread",
        ":timer",
        "//xrtl/testing:gtest_main",
    ],
)

cc_library(
    name = "wait_handle",
    hdrs = ["wait_handle.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)

load("//xrtl/testing:portable_testing.bzl", "portable_test_suite")

portable_test_suite()
