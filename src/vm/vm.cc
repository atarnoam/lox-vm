#include "vm.h"

#include "src/debug_flags.h"
#include "src/vm/debug.h"

InterpretResult VM::interpret(Chunk &ch) {
    chunk = &ch;
    ip = ch.code.begin();
    return run();
}

InterpretResult VM::run() {
    for (;;) {
        if constexpr (DEBUG_TRACE_EXECEUTION) {
            disassemble_instruction(*chunk, ip - chunk->code.begin());
        }
        OpCode instruction;
        switch (instruction = read_byte().opcode) {
        case OpCode::OP_RETURN:
            return InterpretResult::INTERPRET_OK;
        case OpCode::OP_CONSTANT:
            Value constant = read_constant();
            std::cout << to_string(constant) << std::endl;
            break;
        }
    }
}

InstructionData VM::read_byte() { return *(ip++); }

Value VM::read_constant() { return chunk->constants[read_byte().constant_ref]; }
