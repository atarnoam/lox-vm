#pragma once

#include "src/vm/chunk.h"
#include "src/vm/value.h"

enum struct InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

struct VM {
    VM() = default;

    InterpretResult interpret(Chunk &chunk);
    InterpretResult run();

  private:
    InstructionData read_byte();
    Value read_constant();

    Chunk *chunk;
    decltype(chunk->code)::const_iterator ip;
};