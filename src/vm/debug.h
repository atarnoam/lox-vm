#pragma once

#include "src/vm/chunk.h"

#include <fmt/format.h>

void disassemble_chunk(Chunk &chunk, const std::string &name);
int disassemble_instruction(Chunk &chunk, int offset);
