#include "vm.h"

#include "src/compiler/compiler.h"
#include "src/debug_flags.h"
#include "src/vm/debug.h"

#include <optional>

VM::VM(InterpretMode interpret_mode) : m_interpret_mode(interpret_mode) {}

InterpretResult VM::run_script(heap_ptr<ObjFunction> main) {
    frames.emplace_back(main, main->chunk.code.begin(), stack.begin());
    stack.emplace_back(main);
    InterpretResult result = run();
    frames.pop_back();
    if (result == InterpretResult::OK) {
        stack.pop_back();
    }
    return result;
}

InterpretResult VM::run() {
#define RETURN_ERROR()                                                         \
    {                                                                          \
        result = InterpretResult::RUNTIME_ERROR;                               \
        goto DONE;                                                             \
    }
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

    CallFrame &frame = frames.back();
    InterpretResult result;

    // Breaking out of this loop is done using `goto`, because we don't want to
    // check a variable in every loop iteration.
    for (;;) {
        if constexpr (DEBUG_TRACE_EXECUTION) {
            print_stack();
            disassemble_instruction(frame.chunk(),
                                    frame.ip - frame.chunk().code.begin());
        }
        OpCode instruction;
        // TODO: understand if functions here are inlined.
        switch (instruction = read_byte(frame).opcode) {
        case OpCode::RETURN:
            result = InterpretResult::OK;
            goto DONE;
            break;
        case OpCode::CONSTANT: {
            Value constant = read_constant(frame);
            push(constant);
        } break;
        case OpCode::DEFINE_GLOBAL: {
            auto name = read_string(frame);
            globals.emplace(name, peek(0));
            pop();
        } break;
        case OpCode::GET_LOCAL: {
            const_ref_t slot = read_byte(frame).constant_ref;
            emplace(stack[slot]);
        } break;
        case OpCode::SET_LOCAL: {
            const_ref_t slot = read_byte(frame).constant_ref;
            stack[slot] = peek(0);
        } break;
        case OpCode::GET_GLOBAL: {
            auto name = read_string(frame);
            if (!globals.contains(name)) {
                runtime_error("Undefined variable '{}'.",
                              static_cast<std::string>(*name));
                RETURN_ERROR();
            }
            push(globals.at(name));
        } break;
        case OpCode::SET_GLOBAL: {
            auto name = read_string(frame);
            auto name_it = globals.find(name);
            if (name_it == globals.end()) {
                runtime_error("Undefined variable '{}'.",
                              static_cast<std::string>(*name));
                RETURN_ERROR();
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
                RETURN_ERROR();
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
        case OpCode::JUMP: {
            jump_off_t offset = read_jump(frame);
            frame.ip += offset;
        } break;
        case OpCode::JUMP_IF_FALSE: {
            jump_off_t offset = read_jump(frame);
            if (!static_cast<bool>(peek(0))) {
                frame.ip += offset;
            }
        } break;
        case OpCode::JUMP_IF_TRUE: {
            jump_off_t offset = read_jump(frame);
            if (static_cast<bool>(peek(0))) {
                frame.ip += offset;
            }
        } break;
        case OpCode::LOOP: {
            jump_off_t offset = read_jump(frame);
            frame.ip -= offset;
        } break;
        }
    }
DONE:
    return result;
#undef ASSERT_NUM
#undef ASSERT_NUMS
#undef RETURN_ERROR
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

jump_off_t VM::read_jump(CallFrame &frame) {
    jump_off_t ret = get_jump_off(frame.ip);
    frame.ip += sizeof(jump_off_t);
    return ret;
}

Value VM::read_constant(CallFrame &frame) {
    return frame.chunk().constants[read_byte(frame).constant_ref];
}

heap_ptr<ObjString> VM::read_string(CallFrame &frame) {
    return read_constant(frame).as_string();
}

void VM::print_stack() const {
    std::cout << "          ";
    for (const auto &value : stack) {
        std::cout << "[ " << value << " ]";
    }
    std::cout << "\n";
}

InterpretResult interpret(VM &vm, const std::string &source) {
    Parser parser{source};
    Compiler compiler{vm.get_heap_manager(), parser};
    auto func_opt = compiler.compile();

    if (!func_opt.has_value()) {
        return InterpretResult::COMPILE_ERROR;
    }

    InterpretResult result = vm.run_script(func_opt.value());

    return result;
}

Chunk &CallFrame::chunk() { return function->chunk; }
