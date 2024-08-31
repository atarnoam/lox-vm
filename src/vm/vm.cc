#include "vm.h"

#include "src/compiler/compiler.h"
#include "src/debug_flags.h"
#include "src/vm/debug.h"

#include <optional>

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
        if constexpr (DEBUG_TRACE_EXECEUTION) {
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
            Value popped = pop();
            std::cout << popped << "\n";
            return InterpretResult::OK;
        }
        case OpCode::CONSTANT: {
            Value constant = read_constant();
            push(constant);
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
            ASSERT_NUMS();
            binary_func<double, std::plus>();
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

void VM::reset_stack() { stack.resize(0); }

Value VM::read_constant() { return chunk->constants[read_byte().constant_ref]; }

InterpretResult interpret(VM &vm, const std::string &source) {
    Compiler compiler(source);
    std::optional<Chunk> chunk = compiler.compile();

    if (!chunk.has_value()) {
        return InterpretResult::COMPILE_ERROR;
    }

    InterpretResult result = vm.run_chunk(chunk.value());

    return result;
}
