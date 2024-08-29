#include <gtest/gtest.h>

#include "src/vm/chunk.h"

TEST(ChunkTests, TestLineData) {
    Chunk chunk;
    int const_ref = chunk.add_constant(1.2);

    for (int line = 1; line <= 10; ++line) {
        chunk.write_chunk(OpCode::OP_CONSTANT, line);
        chunk.write_chunk(const_ref, line);
    }

    for (int instruction = 18; instruction >= 0; --instruction) {
        int line = chunk.get_line(instruction);
        EXPECT_EQ(line, instruction / 2 + 1);
    }

    for (int instruction = 0; instruction <= 18; ++instruction) {
        int line = chunk.get_line(instruction);
        EXPECT_EQ(line, instruction / 2 + 1);
    }
}