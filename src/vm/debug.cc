#include "debug.h"

#include <fmt/format.h>
#include <iostream>

void disassemble_chunk(Chunk &chunk, const std::string &name) {
    std::cout << fmt::format("== {} ==\n", name);
    for (int offset = 0; offset < static_cast<int>(chunk.code.size());) {
        offset = disassemble_instruction(chunk, offset);
    }
}

int simple_instruction(const std::string &name, int offset) {
    std::cout << fmt::format("{}\n", name);
    return offset + 1;
}

int constant_instruction(const std::string &name, Chunk &chunk, int offset) {
    ConstRefT constant = chunk.code[offset + 1].constant_ref;
    std::cout << fmt::format("{:16s} {:4d} '", name, constant);
    std::cout << to_string(chunk.constants[constant]) << "'\n";
    return offset + 2;
}

int disassemble_instruction(Chunk &chunk, int offset) {
    std::cout << fmt::format("{:04d} ", offset);
    if (offset > 0 and chunk.get_line(offset - 1) == chunk.get_line(offset)) {
        std::cout << "   | ";
    } else {
        std::cout << fmt::format("{:4d} ", chunk.get_line(offset));
    }
    OpCode instruction = chunk.code[offset].opcode;
    switch (instruction) {
    case OpCode::OP_RETURN:
        return simple_instruction("OP_RETURN", offset);
    case OpCode::OP_CONSTANT:
        return constant_instruction("OP_CONSTANT", chunk, offset);
    case OpCode::OP_ADD:
        return simple_instruction("OP_ADD", offset);
    case OpCode::OP_SUBTRACT:
        return simple_instruction("OP_SUBTRACT", offset);
    case OpCode::OP_MULTIPLY:
        return simple_instruction("OP_MULTIPLY", offset);
    case OpCode::OP_DIVIDE:
        return simple_instruction("OP_DIVIDE", offset);
    case OpCode::OP_NEGATE:
        return simple_instruction("OP_NEGATE", offset);
    default:
        std::cout << fmt::format("Unknown opcode {}\n",
                                 static_cast<int>(instruction));
        return offset + 1;
    }
}
