#include <iostream>

#include "src/vm/chunk.h"
#include "src/vm/debug.h"

int main() {
    Chunk chunk;
    int const_ref = chunk.add_constant(1.2);

    for (int line = 1; line <= 10; ++line) {
        chunk.write_chunk(OpCode::OP_CONSTANT, line);
        chunk.write_chunk(const_ref, line);
    }

    disassemble_chunk(chunk, "test_chunk");
}