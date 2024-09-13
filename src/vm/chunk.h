#pragma once

#include "src/vm/code.h"
#include "src/vm/value.h"

/** This is a chunk with initialized constant `Value`s.
 */
struct Chunk : CodeChunk {
    Chunk() = default;

    int add_constant(Value value);

    std::vector<Value> constants;
};
