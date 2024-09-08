package(default_visibility = ["//visibility:public"])

cc_library(
    name = "heap",
    srcs = ["heap.cc"],
    hdrs = ["heap.h"],
    deps = [":heap_obj"],
)

cc_library(
    name = "heap_obj",
    srcs = ["heap_obj.cc"],
    hdrs = ["heap_obj.h"],
    deps = [],
)

cc_test(
    name = "heap_test",
    size = "small",
    srcs = ["heap_test.cc"],
    deps = [
        ":heap",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
