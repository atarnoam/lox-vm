#pragma once

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

enum struct OpCode : int8_t {
    CONSTANT,
    NIL,
    TRUE,
    FALSE,
    EQUAL,
    GREATER,
    LESS,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    NOT,
    NEGATE,
    POP,
    DEFINE_GLOBAL,
    SET_GLOBAL,
    GET_GLOBAL,
    PRINT,
    RETURN
};

using const_ref_t = int8_t;

union InstructionData {
    OpCode opcode;
    const_ref_t constant_ref;

    InstructionData(OpCode opcode);
    InstructionData(const_ref_t constant_ref);
};

/** A code chunk is a chunk that holds just code, and the line numbers they came
 * from. This is a syntactic type, and thus `Value`s are not held here.
 */
struct CodeChunk {

    CodeChunk() = default;

    void write(InstructionData instruction_data, int line);

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

  private:
    LineData lines;
};
