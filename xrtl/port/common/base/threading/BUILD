# Description:
#  Posix pthreads port variants of //xrtl/base/threading/ features.

package(default_visibility = ["//xrtl:internal"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "epoll_message_loop",
    srcs = ["epoll_message_loop.cc"],
    hdrs = ["epoll_message_loop.h"],
    deps = [
        "//xrtl/base:system_clock",
        "//xrtl/base/threading:event",
        "//xrtl/base/threading:thread",
        "//xrtl/base/threading:threading_hdrs",
    ],
)

cc_library(
    name = "pthreads_event",
    srcs = ["pthreads_event.cc"],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        ":pthreads_wait_handle",
        "//xrtl/base/threading:threading_hdrs",
    ],
)

cc_library(
    name = "pthreads_semaphore",
    srcs = ["pthreads_semaphore.cc"],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        ":pthreads_wait_handle",
        "//xrtl/base/threading:threading_hdrs",
    ],
)

cc_library(
    name = "pthreads_thread",
    srcs = ["pthreads_thread.cc"],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        ":pthreads_event",
        ":pthreads_wait_handle",
        "//xrtl/base:logging",
        "//xrtl/base:stopwatch",
        "//xrtl/base/threading:thread_base",
        "//xrtl/base/threading:threading_hdrs",
        "//xrtl/tools/target_platform",
    ],
)

cc_library(
    name = "pthreads_wait_handle",
    srcs = ["pthreads_wait_handle.cc"],
    hdrs = ["pthreads_wait_handle.h"],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//xrtl/base/threading:wait_handle",
    ],
)
