#pragma once

#include "src/vm/obj_function.fwd.h"

#include "src/vm/chunk.h"

struct ObjFunction {
    int arity;
    Chunk chunk;
    heap_ptr<ObjString> name;
};