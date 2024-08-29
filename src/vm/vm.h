#pragma once

#include "src/vm/chunk.h"
#include "src/vm/value.h"

#include <functional>

enum struct InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

struct VM {
    VM() = default;

    InterpretResult interpret(Chunk &chunk);
    InterpretResult run();

    void push(Value value);
    Value pop();

  private:
    InstructionData read_byte();
    Value read_constant();

    template <typename FT, FT F>
    void binary_func() {
        Value b = pop();
        Value a = pop();
        push(F(a, b));
    }

    Chunk *chunk;
    decltype(chunk->code)::const_iterator ip;
    std::vector<Value> stack;
};