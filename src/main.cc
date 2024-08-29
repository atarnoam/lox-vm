#include <iostream>

#include "src/vm/chunk.h"
#include "src/vm/debug.h"
#include "src/vm/vm.h"

int main() {
    VM vm{};
    Chunk chunk;
    int const1_ref = chunk.add_constant(1.2);
    int const2_ref = chunk.add_constant(1.8);
    int const3_ref = chunk.add_constant(3.2);

    chunk.write_chunk(OpCode::OP_CONSTANT, 1);
    chunk.write_chunk(const1_ref, 1);
    chunk.write_chunk(OpCode::OP_CONSTANT, 1);
    chunk.write_chunk(const2_ref, 1);
    chunk.write_chunk(OpCode::OP_CONSTANT, 1);
    chunk.write_chunk(const3_ref, 1);

    chunk.write_chunk(OpCode::OP_DIVIDE, 123);
    chunk.write_chunk(OpCode::OP_ADD, 123);

    chunk.write_chunk(OpCode::OP_RETURN, 123);

    vm.interpret(chunk);
}