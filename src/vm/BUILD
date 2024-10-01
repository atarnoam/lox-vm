package(default_visibility = ["//visibility:public"])

cc_library(
    name = "code",
    srcs = ["code.cc"],
    hdrs = ["code.h"],
)

cc_library(
    name = "chunk",
    srcs = ["chunk.cc"],
    hdrs = ["chunk.h"],
    deps = [
        ":code",
        ":value_h",
    ],
)

cc_library(
    name = "debug",
    srcs = ["debug.cc"],
    hdrs = ["debug.h"],
    deps = [
        ":chunk",
        ":obj_function",
        ":value",
        "@fmt",
    ],
)

cc_library(
    name = "value_h",
    hdrs = ["value.h"],
    deps = [
        "//src/vm:obj_function_fwd",
        "//src/vm:object",
        "//src/vm/gc:heap_obj",
        "@fmt",
    ],
)

cc_library(
    name = "value",
    srcs = ["value.cc"],
    deps = [
        "//src/vm:obj_function",
        "//src/vm:object",
        "//src/vm:value_h",
        "//src/vm/gc:heap_obj",
        "@fmt",
    ],
)

cc_test(
    name = "value_test",
    size = "small",
    srcs = ["value_test.cc"],
    deps = [
        ":value",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "heap_manager",
    srcs = ["heap_manager.cc"],
    hdrs = ["heap_manager.h"],
    deps = [
        "//src/vm:obj_function",
        "//src/vm:value",
        "//src/vm/gc:heap",
    ],
)

cc_library(
    name = "vm",
    srcs = ["vm.cc"],
    hdrs = ["vm.h"],
    deps = [
        ":chunk",
        ":debug",
        ":heap_manager",
        ":natives",
        ":object",
        ":value",
        "//src:debug_flags",
        "//src/compiler",
        "//src/vm:obj_function",
        "//src/vm/gc:heap",
        "//src/vm/gc:heap_obj",
        "@fmt",
    ],
)

cc_test(
    name = "chunk_test",
    size = "small",
    srcs = ["chunk_test.cc"],
    deps = [
        ":chunk",
        ":value",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "object",
    srcs = ["object.cc"],
    hdrs = ["object.h"],
    deps = [
        "//src/vm/gc:heap_obj",
    ],
)

cc_library(
    name = "obj_function_fwd",
    hdrs = ["obj_function.fwd.h"],
    deps = [
    ],
)

cc_library(
    name = "obj_function",
    srcs = ["obj_function.cc"],
    hdrs = ["obj_function.h"],
    deps = [
        "//src/vm:chunk",
        "//src/vm:obj_function_fwd",
        "//src/vm:object",
        "//src/vm/gc:heap_obj",
    ],
)

cc_library(
    name = "natives",
    srcs = ["natives.cc"],
    hdrs = ["natives.h"],
    deps = [
        "//src/vm:object",
        "//src/vm:value",
    ],
)
