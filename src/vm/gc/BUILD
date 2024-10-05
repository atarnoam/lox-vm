package(default_visibility = ["//visibility:public"])

cc_library(
    name = "heap",
    srcs = ["heap.cc"],
    hdrs = ["heap.h"],
    deps = [
        ":heap_obj",
        "//src:debug_flags",
        "//src/util:type_name",
        "@fmt",
    ],
)

cc_library(
    name = "heap_obj",
    srcs = ["heap_obj.cc"],
    hdrs = ["heap_obj.h"],
    deps = [
        "//src:debug_flags",
        "//src/util:type_name",
        "@fmt",
    ],
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

cc_library(
    name = "gc_allocator",
    hdrs = ["gc_allocator.h"],
    deps = ["//src:debug_flags"],
)

cc_library(
    name = "gc_type",
    hdrs = ["gc_type.h"],
    deps = [],
)
