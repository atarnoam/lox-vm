#pragma once

#include "src/vm/chunk.h"
#include "src/vm/value.h"

#include <functional>

enum struct InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

struct VM {
    VM() = default;

    InterpretResult run_chunk(Chunk &chunk);
    InterpretResult run();

    void push(Value value);
    Value pop();

  private:
    __attribute__((always_inline)) InstructionData read_byte() {
        return *(ip++);
    }
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

InterpretResult interpret(VM &vm, const std::string &source);
