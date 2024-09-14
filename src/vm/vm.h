#pragma once

#include "src/vm/chunk.h"
#include "src/vm/gc/heap.h"
#include "src/vm/gc/heap_obj.h"
#include "src/vm/heap_manager.h"
#include "src/vm/obj_function.h"
#include "src/vm/object.h"
#include "src/vm/value.h"

#include <fmt/format.h>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

constexpr int FRAMES_MAX = 64;

enum struct InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };
enum struct InterpretMode { FILE, INTERACTIVE };

using Stack = std::vector<Value>;

struct CallFrame {
    Chunk &chunk();

    heap_ptr<ObjFunction> function;
    CodeVec::const_iterator ip;
    Stack::const_iterator slots;
};

struct VM {
    VM(InterpretMode interpret_mode);

    InterpretResult run_script(heap_ptr<ObjFunction> main);
    InterpretResult run();

    void push(const Value &value);
    template <typename... Args>
    void emplace(Args &&...args) {
        stack.emplace_back(args...);
    }
    Value pop();
    Value peek(int distance = 0) const;

    HeapManager &get_heap_manager();
    InterpretMode interpret_mode() const;

  private:
    void reset_stack();

    __attribute__((always_inline)) InstructionData read_byte(CallFrame &frame) {
        return *(frame.ip++);
    }
    jump_off_t read_jump(CallFrame &frame);
    Value read_constant(CallFrame &frame);
    heap_ptr<ObjString> read_string(CallFrame &frame);

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
        CallFrame &frame = frames.back();
        Chunk &chunk = frame.chunk();

        std::cerr << fmt::format(fmt, std::forward<Args>(args)...) << '\n';
        int instruction = frame.ip - chunk.code.begin() - 1;
        int line = chunk.get_line(instruction);
        std::cerr << fmt::format("[line {}] in script.", line) << std::endl;
        reset_stack();
    };

    void print_stack() const;

    HeapManager heap_manager;
    VariableMap globals;
    Stack stack;
    InterpretMode m_interpret_mode;
    std::vector<CallFrame> frames;
};

InterpretResult interpret(VM &vm, const std::string &source);
