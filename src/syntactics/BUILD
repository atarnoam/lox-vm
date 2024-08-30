package(default_visibility = ["//visibility:public"])

cc_library(
    name = "token",
    srcs = ["token.cc"],
    hdrs = ["token.h"],
    deps = [
    ],
)

cc_library(
    name = "scanner",
    srcs = ["scanner.cc"],
    hdrs = ["scanner.h"],
    deps = [
        ":token",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":logging",
        ":scanner",
        ":token",
    ],
)

cc_library(
    name = "logging",
    srcs = ["logging.cc"],
    hdrs = ["logging.h"],
    deps = [
        ":token",
        "@fmt",
    ],
)
