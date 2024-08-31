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
        case OpCode::ADD:
            if (!assert_number(0, 1)) {
                return InterpretResult::RUNTIME_ERROR;
            }
            binary_func<std::plus<double>, std::plus<double>{}>();
            break;
        case OpCode::SUBTRACT:
            if (!assert_number(0, 1)) {
                return InterpretResult::RUNTIME_ERROR;
            }
            binary_func<std::minus<double>, std::minus<double>{}>();
            break;
        case OpCode::MULTIPLY:
            if (!assert_number(0, 1)) {
                return InterpretResult::RUNTIME_ERROR;
            }
            binary_func<std::multiplies<double>, std::multiplies<double>{}>();
            break;
        case OpCode::DIVIDE:
            if (!assert_number(0, 1)) {
                return InterpretResult::RUNTIME_ERROR;
            }
            binary_func<std::divides<double>, std::divides<double>{}>();
            break;
        case OpCode::NEGATE:
            if (!assert_number(0)) {
                return InterpretResult::RUNTIME_ERROR;
            }
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

bool VM::assert_number(int distance) {
    if (!peek(distance).is_number()) {
        runtime_error("Operand must be a number.");
        return false;
    }
    return true;
}

bool VM::assert_number(int distance1, int distance2) {
    if (!peek(distance1).is_number() or !peek(distance2).is_number()) {
        runtime_error("Operands must be numbers.");
        return false;
    }
    return true;
}

InterpretResult interpret(VM &vm, const std::string &source) {
    Compiler compiler(source);
    std::optional<Chunk> chunk = compiler.compile();

    if (!chunk.has_value()) {
        return InterpretResult::COMPILE_ERROR;
    }

    InterpretResult result = vm.run_chunk(chunk.value());

    return result;
}
