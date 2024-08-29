#include <iostream>

#include "src/vm/chunk.h"
#include "src/vm/debug.h"
#include "src/vm/vm.h"

int main() {
    VM vm{};
    Chunk chunk;
    int const_ref = chunk.add_constant(1.2);
    chunk.write_chunk(OpCode::OP_CONSTANT, 1);
    chunk.write_chunk(const_ref, 1);
    chunk.write_chunk(OpCode::OP_RETURN, 1);

    vm.interpret(chunk);
}