package(default_visibility = ["//visibility:public"])

cc_library(
    name = "compiler",
    srcs = ["compiler.cc"],
    hdrs = ["compiler.h"],
    deps = [
        "//src:debug_flags",
        "//src/syntactics:parser",
        "//src/vm:chunk",
        "//src/vm:debug",
        "//src/vm:heap_manager",
        "//src/vm:obj_function",
        "//src/vm:value",
        "@fmt",
    ],
)
