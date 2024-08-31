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
    case OpCode::RETURN:
        return simple_instruction("RETURN", offset);
    case OpCode::CONSTANT:
        return constant_instruction("CONSTANT", chunk, offset);
    case OpCode::ADD:
        return simple_instruction("ADD", offset);
    case OpCode::SUBTRACT:
        return simple_instruction("SUBTRACT", offset);
    case OpCode::MULTIPLY:
        return simple_instruction("MULTIPLY", offset);
    case OpCode::DIVIDE:
        return simple_instruction("DIVIDE", offset);
    case OpCode::NEGATE:
        return simple_instruction("NEGATE", offset);
    default:
        std::cout << fmt::format("Unknown opcode {}\n",
                                 static_cast<int>(instruction));
        return offset + 1;
    }
}
