package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        "//src/compiler",
        "//src/vm",
    ],
)

cc_library(
    name = "debug_flags",
    hdrs = ["debug_flags.h"],
)
