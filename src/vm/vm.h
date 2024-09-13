#pragma once

#include "src/vm/chunk.h"
#include "src/vm/gc/heap.h"
#include "src/vm/gc/heap_obj.h"
#include "src/vm/heap_manager.h"
#include "src/vm/obj_string.h"
#include "src/vm/value.h"

#include <fmt/format.h>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
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

    HeapManager &get_heap_manager();

  private:
    void reset_stack();

    __attribute__((always_inline)) InstructionData read_byte() {
        return *(ip++);
    }
    Value read_constant();

    template <typename T, template <typename S> typename FT>
    void binary_func() {
        T b = static_cast<T>(pop());
        T a = static_cast<T>(pop());
        if constexpr (std::is_same_v<T, heap_ptr<ObjString>>) {
            emplace(heap_manager.initialize(FT<std::string>{}(*a, *b)));
        } else {
            emplace(FT{}(a, b));
        }
    }

    template <typename... Args>
    void runtime_error(fmt::format_string<Args...> fmt, Args &&...args) {
        std::cerr << fmt::format(fmt, std::forward<Args>(args)...) << '\n';
        int instruction = ip - chunk->code.begin() - 1;
        int line = chunk->get_line(instruction);
        std::cerr << fmt::format("[line {}] in script.", line) << std::endl;
        reset_stack();
    };

    HeapManager heap_manager;
    Chunk *chunk;
    decltype(chunk->code)::const_iterator ip;
    std::vector<Value> stack;
};

InterpretResult interpret(VM &vm, const std::string &source);
