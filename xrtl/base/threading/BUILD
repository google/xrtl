# Description:
#  Threading library.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

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
    ],
)

cc_library(
    name = "threading_hdrs",
    hdrs = [
        "event.h",
        "semaphore.h",
        "thread.h",
        "timer.h",
    ],
    visibility = ["//xrtl/port:ports_only"],
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

cc_test(
    name = "event_test",
    srcs = ["event_test.cc"],
    deps = [
        ":event",
        ":thread",
        "//xrtl/base:stopwatch",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
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

cc_test(
    name = "semaphore_test",
    srcs = ["semaphore_test.cc"],
    deps = [
        ":event",
        ":semaphore",
        ":thread",
        "//xrtl/base:stopwatch",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "thread",
    hdrs = ["thread.h"],
    deps = [
        ":wait_handle",
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

cc_test(
    name = "thread_test",
    srcs = ["thread_test.cc"],
    deps = [
        ":event",
        ":semaphore",
        ":thread",
        "//xrtl/base:stopwatch",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
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

cc_test(
    name = "timer_test",
    srcs = ["timer_test.cc"],
    deps = [
        ":timer",
        ":thread",
        "//xrtl/testing:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "wait_handle",
    hdrs = ["wait_handle.h"],
    deps = [
        "//xrtl/base:ref_ptr",
    ],
)
