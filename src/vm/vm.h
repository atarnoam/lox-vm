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
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

constexpr int FRAMES_MAX = 64;

enum struct InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };
enum struct InterpretMode { FILE, INTERACTIVE };

using Stack = std::vector<Value>;

struct CallFrame {
    CallFrame(heap_ptr<ObjFunction> function, CodeVec::const_iterator ip,
              size_t slots);
    Chunk &chunk();

    heap_ptr<ObjFunction> function;
    CodeVec::const_iterator ip;
    const size_t slots;
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

    __attribute__((always_inline)) InstructionData read_byte(CallFrame *frame) {
        return *(frame->ip++);
    }
    jump_off_t read_jump(CallFrame *frame);
    Value read_constant(CallFrame *frame);
    heap_ptr<ObjString> read_string(CallFrame *frame);

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

    bool call_value(const Value &callee, int arg_count);
    bool call(heap_ptr<ObjFunction> function, int arg_count);

    template <typename... Args>
    void runtime_error(fmt::format_string<Args...> fmt, Args &&...args) {
        std::cerr << fmt::format(fmt, std::forward<Args>(args)...) << '\n';

        for (const auto &frame : std::ranges::reverse_view(frames)) {
            auto function = frame.function;
            size_t instruction = frame.ip - function->chunk.code.begin() - 1;
            std::cerr << fmt::format("[line {}] in ",
                                     function->chunk.get_line(instruction));
            if (function->name == nullptr) {
                std::cerr << "script\n";
            } else {
                std::cerr << fmt::format("{}()\n", function->name->str());
            }
        }
        reset_stack();
        frames.clear();
    };

    void print_stack() const;

    HeapManager heap_manager;
    VariableMap globals;
    Stack stack;
    InterpretMode m_interpret_mode;
    std::vector<CallFrame> frames;
};

InterpretResult interpret(VM &vm, const std::string &source);
