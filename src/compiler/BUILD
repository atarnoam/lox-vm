package(default_visibility = ["//visibility:public"])

cc_library(
    name = "compiler",
    srcs = ["compiler.cc"],
    hdrs = ["compiler.h"],
    deps = [
        ":uninitialized_chunk",
        "//src:debug_flags",
        "//src/syntactics:parser",
        "//src/vm:chunk",
        "//src/vm:debug",
        "//src/vm:value",
        "@fmt",
    ],
)

cc_library(
    name = "uninitialized_chunk",
    hdrs = ["uninitialized_chunk.h"],
    deps = [
        "//src/vm:code",
        "//src/vm:value",
        "//src/vm/gc:heap",
    ],
)

cc_library(
    name = "initializer",
    srcs = ["initializer.cc"],
    hdrs = ["initializer.h"],
    deps = [
        "//src/vm:value",
        "//src/vm/gc:heap",
    ],
)
