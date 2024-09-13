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
    SET_LOCAL,
    GET_LOCAL,
    PRINT,
    JUMP,
    JUMP_IF_FALSE,
    JUMP_IF_TRUE,
    RETURN
};

using const_ref_t = int8_t;
using jump_off_t = uint16_t;

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

    template <typename IteratorT>
    jump_off_t read_jump(const IteratorT &it) const {
        return read_jump_ptr(&(*it));
    }

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
    jump_off_t read_jump_ptr(const InstructionData *code_ptr) const;

    LineData lines;
};

template <>
jump_off_t CodeChunk::read_jump<int>(const int &offset) const;
