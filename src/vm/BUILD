package(default_visibility = ["//visibility:public"])

cc_library(
    name = "chunk",
    srcs = ["chunk.cc"],
    hdrs = ["chunk.h"],
    deps = [":value"],
)

cc_library(
    name = "debug",
    srcs = ["debug.cc"],
    hdrs = ["debug.h"],
    deps = [
        ":chunk",
        ":value",
        "@fmt",
    ],
)

cc_library(
    name = "value",
    srcs = ["value.cc"],
    hdrs = ["value.h"],
    deps = [
        "@fmt",
    ],
)

cc_library(
    name = "vm",
    srcs = ["vm.cc"],
    hdrs = ["vm.h"],
    deps = [
        ":chunk",
        ":debug",
        ":value",
        "//src:debug_flags",
    ],
)

cc_test(
    name = "chunk_test",
    size = "small",
    srcs = ["chunk_test.cc"],
    deps = [
        ":chunk",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
