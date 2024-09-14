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
    LOOP,
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

inline const jump_off_t *as_jump_ptr(const InstructionData *code_ptr) {
    return reinterpret_cast<const jump_off_t *>(code_ptr);
}

inline jump_off_t *as_jump_ptr(InstructionData *code_ptr) {
    return reinterpret_cast<jump_off_t *>(code_ptr);
}

template <typename PtrT>
jump_off_t get_jump_off(const PtrT &it) {
    return *as_jump_ptr(&(*it));
}

using CodeVec = std::vector<InstructionData>;

/** A code chunk is a chunk that holds just code, and the line numbers they
 * came from. This is a syntactic type, and thus `Value`s are not held here.
 */
struct CodeChunk {
    CodeChunk() = default;

    CodeChunk(const CodeChunk &other) = delete;
    CodeChunk &operator=(const CodeChunk &other) = delete;

    CodeChunk(CodeChunk &&other) = default;
    CodeChunk &operator=(CodeChunk &&other) = default;

    virtual ~CodeChunk() = default;

    void write(InstructionData instruction_data, int line);

    int get_line(int instruction);

    const jump_off_t &jump_at(int offset) const;
    jump_off_t &jump_at(int offset);

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
    CodeVec code;

  private:
    LineData lines;
};
