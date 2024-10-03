#pragma once

#include "src/vm/obj_function.fwd.h"

#include "src/vm/chunk.h"
#include "src/vm/obj_upvalue.fwd.h"

struct ObjFunction {
    ObjFunction(int arity, heap_ptr<ObjString> name);

    int arity;
    heap_ptr<ObjString> name;
    const_ref_t upvalue_count;
    Chunk chunk;
};

struct ObjClosure {
    ObjClosure(heap_ptr<ObjFunction> function);

    heap_ptr<ObjFunction> function;
    std::vector<heap_ptr<ObjUpvalue>> upvalues;
    const_ref_t upvalue_count;
};