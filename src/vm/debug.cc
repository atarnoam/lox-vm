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
    const_ref_t constant = chunk.code[offset + 1].constant_ref;
    std::cout << fmt::format("{:16s} {:4d} '", name, constant);
    std::cout << chunk.constants[constant] << "'\n";
    return offset + 2;
}

int byte_instruction(const std::string &name, Chunk &chunk, int offset) {
    const_ref_t slot = chunk.code[offset + 1].constant_ref;
    std::cout << fmt::format("{:16s} {:4d}\n", name, slot);
    return offset + 2;
}

int jump_instruction(const std::string &name, int sign, const Chunk &chunk,
                     int offset) {
    jump_off_t jump = chunk.jump_at(offset + 1);
    std::cout << fmt::format("{:16s} {:4d} -> {:d}\n", name, offset,
                             offset + 3 + sign * jump);
    return offset + 3;
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
    case OpCode::DEFINE_GLOBAL:
        return constant_instruction("DEFINE_GLOBAL", chunk, offset);
    case OpCode::GET_LOCAL:
        return byte_instruction("GET_LOCAL", chunk, offset);
    case OpCode::SET_LOCAL:
        return byte_instruction("SET_LOCAL", chunk, offset);
    case OpCode::GET_GLOBAL:
        return constant_instruction("GET_GLOBAL", chunk, offset);
    case OpCode::SET_GLOBAL:
        return constant_instruction("SET_GLOBAL", chunk, offset);
    case OpCode::ADD:
        return simple_instruction("ADD", offset);
    case OpCode::SUBTRACT:
        return simple_instruction("SUBTRACT", offset);
    case OpCode::MULTIPLY:
        return simple_instruction("MULTIPLY", offset);
    case OpCode::DIVIDE:
        return simple_instruction("DIVIDE", offset);
    case OpCode::NOT:
        return simple_instruction("NOT", offset);
    case OpCode::NEGATE:
        return simple_instruction("NEGATE", offset);
    case OpCode::NIL:
        return simple_instruction("NIL", offset);
    case OpCode::TRUE:
        return simple_instruction("TRUE", offset);
    case OpCode::FALSE:
        return simple_instruction("FALSE", offset);
    case OpCode::EQUAL:
        return simple_instruction("EQUAL", offset);
    case OpCode::GREATER:
        return simple_instruction("GREATER", offset);
    case OpCode::LESS:
        return simple_instruction("LESS", offset);
    case OpCode::POP:
        return simple_instruction("POP", offset);
    case OpCode::PRINT:
        return simple_instruction("PRINT", offset);
    case OpCode::JUMP:
        return jump_instruction("JUMP", 1, chunk, offset);
    case OpCode::JUMP_IF_FALSE:
        return jump_instruction("JUMP_IF_FALSE", 1, chunk, offset);
    case OpCode::JUMP_IF_TRUE:
        return jump_instruction("JUMP_IF_TRUE", 1, chunk, offset);
    case OpCode::LOOP:
        return jump_instruction("LOOP", -1, chunk, offset);
    case OpCode::CALL:
        return byte_instruction("CALL", chunk, offset);
    default:
        std::cout << fmt::format("Unknown opcode {}\n",
                                 static_cast<int>(instruction));
        return offset + 1;
    }
}
