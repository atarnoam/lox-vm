#include "code.h"

#include <stdexcept>

InstructionData::InstructionData(OpCode opcode) : opcode(opcode) {}

InstructionData::InstructionData(const_ref_t constant_ref)
    : constant_ref(constant_ref) {}

void CodeChunk::write(InstructionData instruction_data, int line) {
    code.push_back(instruction_data);
    lines.add_line(line);
}

int CodeChunk::get_line(int instruction) { return lines.get_line(instruction); }

const jump_off_t &CodeChunk::jump_at(int offset) const {
    return *as_jump_ptr(&code[offset]);
}

jump_off_t &CodeChunk::jump_at(int offset) {
    return *as_jump_ptr(&code[offset]);
}

CodeChunk::LineData::LineData()
    : lines(), last_line_index(0), last_instruction(-1) {}

void CodeChunk::LineData::add_line(int line) {
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

int CodeChunk::LineData::get_line(int instruction) {
    int curr_instruction;
    size_t index;

    if (last_instruction == instruction) {
        return lines[last_line_index].first;
    }
    if (last_instruction >= 0 and last_instruction <= instruction) {
        curr_instruction = last_instruction;
        index = last_line_index;
    } else {
        curr_instruction = 0;
        index = 0;
    }

    for (; index < lines.size(); ++index) {
        const auto &[line, count] = lines[index];
        curr_instruction += count;
        if (instruction < curr_instruction) {
            last_instruction = curr_instruction - count;
            last_line_index = index;
            return line;
        }
    }
    throw std::runtime_error("line of instruction not found");
}
