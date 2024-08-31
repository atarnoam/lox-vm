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
                std::cout << "[ " << to_string(value) << " ]";
            }
            std::cout << "\n";
        }
        OpCode instruction;
        // TODO: understand if functions here are inlined.
        switch (instruction = read_byte().opcode) {
        case OpCode::RETURN:
            std::cout << to_string(pop()) << "\n";
            return InterpretResult::OK;
        case OpCode::CONSTANT: {
            Value constant = read_constant();
            push(constant);
        } break;
        case OpCode::ADD:
            binary_func<std::plus<Value>, std::plus<Value>{}>();
            break;
        case OpCode::SUBTRACT:
            binary_func<std::minus<Value>, std::minus<Value>{}>();
            break;
        case OpCode::MULTIPLY:
            binary_func<std::multiplies<Value>, std::multiplies<Value>{}>();
            break;
        case OpCode::DIVIDE:
            binary_func<std::divides<Value>, std::divides<Value>{}>();
            break;
        case OpCode::NEGATE:
            push(-pop());
            break;
        }
    }
}

void VM::push(Value value) { stack.push_back(value); }

Value VM::pop() {
    Value popped = stack.back();
    stack.pop_back();
    return popped;
}

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
