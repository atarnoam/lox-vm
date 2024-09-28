#pragma once

#include "src/vm/obj_function.fwd.h"

#include "src/vm/chunk.h"

struct ObjFunction {
    ObjFunction(int arity, heap_ptr<ObjString> name);

    int arity;
    heap_ptr<ObjString> name;
    const_ref_t upvalue_count;
    Chunk chunk;
};

struct ObjClosure {
    heap_ptr<ObjFunction> function;
};