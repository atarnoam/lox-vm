package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        "//src/compiler",
        "//src/vm",
        "//src/vm:chunk",
        "//src/vm:debug",
    ],
)

cc_library(
    name = "logging",
    srcs = ["logging.cc"],
    hdrs = ["logging.h"],
    deps = [
        "//src/syntactics:token",
    ],
)

cc_library(
    name = "debug_flags",
    hdrs = ["debug_flags.h"],
)
