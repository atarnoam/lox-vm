#pragma once

#include "src/vm/value.h"

#include <cstdint>
#include <utility>
#include <vector>

enum struct OpCode : int8_t {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN
};

using ConstRefT = int8_t;

union InstructionData {
    OpCode opcode;
    ConstRefT constant_ref;
};

struct Chunk {
    Chunk() = default;

    void write_chunk(InstructionData instruction_data, int line);
    void write_chunk(OpCode opcode, int line);
    void write_chunk(ConstRefT constant_ref, int line);
    int add_constant(Value value);

    int get_line(int instruction);

    struct LineData {
        LineData();

        // Assume monotonicity of lines.
        void add_line(int line);
        int get_line(int instruction);

        std::vector<std::pair<int, int>> lines;

      private:
        // For caching - stores last offset->line gotten.
        decltype(lines)::const_iterator last_line;
        int last_instruction;
    };

    std::vector<InstructionData> code;
    std::vector<Value> constants;

  private:
    LineData lines;
};
