#pragma once

#include "src/vm/chunk.h"
#include "src/vm/value.h"

#include <fmt/format.h>
#include <functional>
#include <utility>

enum struct InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

struct VM {
    VM() = default;

    InterpretResult run_chunk(Chunk &chunk);
    InterpretResult run();

    void push(const Value &value);
    template <typename... Args>
    void emplace(Args &&...args) {
        stack.emplace_back(args...);
    }
    Value pop();
    Value peek(int distance = 0) const;

  private:
    void reset_stack();

    __attribute__((always_inline)) InstructionData read_byte() {
        return *(ip++);
    }
    Value read_constant();

    bool assert_number(int distance);
    bool assert_number(int distance1, int distance2);

    template <typename FT, FT F>
    void binary_func() {
        double b = pop().as_number();
        double a = pop().as_number();
        push(F(a, b));
    }

    template <typename... Args>
    void runtime_error(fmt::format_string<Args...> fmt, Args &&...args) {
        std::cerr << fmt::format(fmt, std::forward<Args>(args)...) << '\n';
        int instruction = ip - chunk->code.begin() - 1;
        int line = chunk->get_line(instruction);
        std::cerr << fmt::format("[line {}] in script", line) << std::endl;
        reset_stack();
    };

    Chunk *chunk;
    decltype(chunk->code)::const_iterator ip;
    std::vector<Value> stack;
};

InterpretResult interpret(VM &vm, const std::string &source);
