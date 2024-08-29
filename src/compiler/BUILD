package(default_visibility = ["//visibility:public"])

cc_library(
    name = "compiler",
    srcs = ["compiler.cc"],
    hdrs = ["compiler.h"],
    deps = [
        "//src/syntactics:scanner",
        "//src/vm:chunk",
        "@fmt",
    ],
)
