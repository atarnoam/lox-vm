#include "chunk.h"

void Chunk::write_chunk(InstructionData instruction_data, int line) {
    code.push_back(instruction_data);
    lines.add_line(line);
}

void Chunk::write_chunk(OpCode opcode, int line) {
    write_chunk({.opcode = opcode}, line);
}

void Chunk::write_chunk(ConstRefT constant_ref, int line) {
    write_chunk({.constant_ref = constant_ref}, line);
}

int Chunk::add_constant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
}

Chunk::LineData::LineData()
    : lines(), last_line(lines.begin() - 1), last_instruction(-1) {}

void Chunk::LineData::add_line(int line) {
    if (lines.size() == 0) {
        lines.push_back({line, 1});
        return;
    }

    auto &[last_line, count] = lines.back();

    if (last_line > line) {
        throw std::runtime_error(
            "Lines are not monotonic! This is not implemented yet.");
    }

    if (last_line == line) {
        ++count;

    } else {
        lines.push_back({line, 1});
    }
}

int Chunk::LineData::get_line(int instruction) {
    int curr_instruction;
    decltype(lines)::const_iterator it;

    if (last_instruction == instruction) {
        return last_line->first;
    }
    if (last_instruction >= 0 and last_instruction <= instruction) {
        curr_instruction = last_instruction;
        it = last_line;
    } else {
        int curr_instruction = 0;
        it = lines.begin();
    }

    for (; it < lines.end(); ++it) {
        const auto &[line, count] = *it;
        curr_instruction += count;
        if (instruction < curr_instruction) {
            last_instruction = curr_instruction - count;
            last_line = it;
            return line;
        }
    }
    throw std::runtime_error("instruction not found");
}
