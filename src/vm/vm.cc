#include "vm.h"

#include "src/compiler/compiler.h"
#include "src/debug_flags.h"
#include "src/vm/debug.h"

#include <optional>

VM::VM(InterpretMode interpret_mode) : m_interpret_mode(interpret_mode) {}

InterpretResult VM::run_chunk(Chunk &ch) {
    chunk = &ch;
    ip = ch.code.begin();
    return run();
}

InterpretResult VM::run() {
#define ASSERT_NUM()                                                           \
    if (!peek(0).is_number()) {                                                \
        runtime_error("Operand must be a number.");                            \
        return InterpretResult::RUNTIME_ERROR;                                 \
    }
#define ASSERT_NUMS()                                                          \
    if (!peek(0).is_number() || !peek(1).is_number()) {                        \
        runtime_error("Operands must be numbers.");                            \
        return InterpretResult::RUNTIME_ERROR;                                 \
    }

    for (;;) {
        if constexpr (DEBUG_TRACE_EXECUTION) {
            disassemble_instruction(*chunk, ip - chunk->code.begin());
            std::cout << "          ";
            for (const auto &value : stack) {
                std::cout << "[ " << value << " ]";
            }
            std::cout << "\n";
        }
        OpCode instruction;
        // TODO: understand if functions here are inlined.
        switch (instruction = read_byte().opcode) {
        case OpCode::RETURN: {
            return InterpretResult::OK;
        }
        case OpCode::CONSTANT: {
            Value constant = read_constant();
            push(constant);
        } break;
        case OpCode::DEFINE_GLOBAL: {
            auto name = read_string();
            globals.emplace(name, peek(0));
            pop();
        } break;
        case OpCode::GET_LOCAL: {
            const_ref_t slot = read_byte().constant_ref;
            emplace(stack[slot]);
        } break;
        case OpCode::SET_LOCAL: {
            const_ref_t slot = read_byte().constant_ref;
            stack[slot] = peek(0);
        } break;
        case OpCode::GET_GLOBAL: {
            auto name = read_string();
            if (!globals.contains(name)) {
                runtime_error("Undefined variable '{}'.",
                              static_cast<std::string>(*name));
                return InterpretResult::RUNTIME_ERROR;
            }
            push(globals.at(name));
        } break;
        case OpCode::SET_GLOBAL: {
            auto name = read_string();
            auto name_it = globals.find(name);
            if (name_it == globals.end()) {
                runtime_error("Undefined variable '{}'.",
                              static_cast<std::string>(*name));
                return InterpretResult::RUNTIME_ERROR;
            }
            auto &[_, value] = *name_it;
            value = peek(0);
        } break;
        case OpCode::EQUAL: {
            Value b = pop();
            Value a = pop();
            emplace(a == b);
        } break;
        case OpCode::GREATER:
            ASSERT_NUMS();
            binary_func<double, std::greater>();
            break;
        case OpCode::LESS:
            ASSERT_NUMS();
            binary_func<double, std::less>();
            break;
        case OpCode::ADD:
            if (peek(0).is_string() and peek(1).is_string()) {
                binary_func<heap_ptr<ObjString>, std::plus>();
            } else if (peek(0).is_number() and peek(1).is_number()) {
                binary_func<double, std::plus>();
            } else {
                runtime_error("Operands must be two numbers or two strings.");
                return InterpretResult::RUNTIME_ERROR;
            }
            break;
        case OpCode::SUBTRACT:
            ASSERT_NUMS();
            binary_func<double, std::minus>();
            break;
        case OpCode::MULTIPLY:
            ASSERT_NUMS();
            binary_func<double, std::multiplies>();
            break;
        case OpCode::DIVIDE:
            ASSERT_NUMS();
            binary_func<double, std::divides>();
            break;
        case OpCode::NOT:
            emplace(!static_cast<bool>(pop()));
            break;
        case OpCode::NEGATE:
            ASSERT_NUM();
            emplace(-pop().as_number());
            break;
        case OpCode::PRINT:
            std::cout << pop() << "\n";
            break;
        case OpCode::POP:
            pop();
            break;
        case OpCode::NIL:
            emplace();
            break;
        case OpCode::TRUE:
            emplace(true);
            break;
        case OpCode::FALSE:
            emplace(false);
            break;
        }
    }
#undef ASSERT_NUM
#undef ASSERT_NUMS
}

void VM::push(const Value &value) { stack.push_back(value); }

Value VM::pop() {
    Value popped = stack.back();
    stack.pop_back();
    return popped;
}

Value VM::peek(int distance) const { return *(stack.rbegin() + distance); }

HeapManager &VM::get_heap_manager() { return heap_manager; }

InterpretMode VM::interpret_mode() const { return m_interpret_mode; }

void VM::reset_stack() { stack.resize(0); }

Value VM::read_constant() { return chunk->constants[read_byte().constant_ref]; }

heap_ptr<ObjString> VM::read_string() { return read_constant().as_string(); }

InterpretResult interpret(VM &vm, const std::string &source) {
    Compiler compiler(vm.get_heap_manager(), source);
    std::optional<Chunk> chunk = compiler.compile();

    if (!chunk.has_value()) {
        return InterpretResult::COMPILE_ERROR;
    }

    InterpretResult result = vm.run_chunk(chunk.value());

    return result;
}
