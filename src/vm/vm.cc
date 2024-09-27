#include "vm.h"

#include "src/compiler/compiler.h"
#include "src/debug_flags.h"
#include "src/vm/debug.h"
#include "src/vm/natives.h"

#include <functional>
#include <optional>

VM::VM(InterpretMode interpret_mode) : m_interpret_mode(interpret_mode) {
    define_all_natives();
}

InterpretResult VM::run_script(heap_ptr<ObjFunction> main) {
    stack.emplace_back(main);
    heap_ptr<ObjClosure> main_closure =
        heap_manager.initialize<ObjClosure>(main);
    pop();
    push(main_closure);
    call(main_closure, 0);
    InterpretResult result = run();
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

    CallFrame *frame = &frames.back();
    InterpretResult result;

    // Breaking out of this loop is done using `goto`, because we don't want to
    // check a variable in every loop iteration.
    for (;;) {
        if constexpr (DEBUG_TRACE_EXECUTION) {
            print_stack();
            disassemble_instruction(frame->chunk(),
                                    frame->ip - frame->chunk().code.begin());
        }
        OpCode instruction;
        // TODO: understand if functions here are inlined.
        switch (instruction = read_byte(frame).opcode) {
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
            emplace(stack[frame->slots + slot]);
        } break;
        case OpCode::SET_LOCAL: {
            const_ref_t slot = read_byte(frame).constant_ref;
            stack[frame->slots + slot] = peek(0);
        } break;
        case OpCode::GET_GLOBAL: {
            auto name = read_string(frame);
            if (!globals.contains(name)) {
                runtime_error("Undefined variable '{}'.", name->str());
                RETURN_ERROR();
            }
            push(globals.at(name));
        } break;
        case OpCode::SET_GLOBAL: {
            auto name = read_string(frame);
            auto name_it = globals.find(name);
            if (name_it == globals.end()) {
                runtime_error("Undefined variable '{}'.", name->str());
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
            frame->ip += offset;
        } break;
        case OpCode::JUMP_IF_FALSE: {
            jump_off_t offset = read_jump(frame);
            if (!static_cast<bool>(peek(0))) {
                frame->ip += offset;
            }
        } break;
        case OpCode::JUMP_IF_TRUE: {
            jump_off_t offset = read_jump(frame);
            if (static_cast<bool>(peek(0))) {
                frame->ip += offset;
            }
        } break;
        case OpCode::LOOP: {
            jump_off_t offset = read_jump(frame);
            frame->ip -= offset;
        } break;
        case OpCode::CALL: {
            const_ref_t arg_count = read_byte(frame).constant_ref;
            // peek(arg_count) is the function that is being called.
            if (!call_value(peek(arg_count), arg_count)) {
                RETURN_ERROR();
            }
            // This is the "jump".
            frame = &frames.back();
        } break;
        case OpCode::CLOSURE: {
            auto function = read_constant(frame).as_function();
            heap_ptr<ObjClosure> closure =
                heap_manager.initialize<ObjClosure>(function);
            push(closure);
        } break;
        case OpCode::RETURN: {
            Value returned = pop();
            size_t last_slot = frame->slots;

            // After this, `frame` is invalidated!
            frames.pop_back();

            if (frames.size() == 0) {
                pop();
                result = InterpretResult::OK;
                goto DONE;
            }
            stack.resize(last_slot);
            push(returned);
            frame = &frames.back();
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

void VM::reset_stack() { stack.clear(); }

jump_off_t VM::read_jump(CallFrame *frame) {
    jump_off_t ret = get_jump_off(frame->ip);
    frame->ip += sizeof(jump_off_t);
    return ret;
}

Value VM::read_constant(CallFrame *frame) {
    return frame->chunk().constants[read_byte(frame).constant_ref];
}

heap_ptr<ObjString> VM::read_string(CallFrame *frame) {
    return read_constant(frame).as_string();
}

void VM::define_native(const std::string &name, const ObjNative &native) {
    globals.emplace(heap_manager.initialize(name),
                    heap_manager.initialize<ObjNative>(native));
}

void VM::define_all_natives() {
    for (const auto &[name, native] : NATIVES) {
        define_native(name, native);
    }
}

bool VM::call_value(const Value &callee, int arg_count) {
    switch (callee.type()) {
    case ValueType::NATIVE:
        return call_native(callee.as_native(), arg_count);
    case ValueType::CLOSURE:
        return call(callee.as_closure(), arg_count);
    // Removed calling of functions, as they are always closed by closures.
    case ValueType::FUNCTION:
        /* fallthrough */
    default:
        break;
    }
    runtime_error("Can only call functions and classes.");
    return false;
}

bool VM::call(heap_ptr<ObjClosure> closure, int arg_count) {
    auto function = closure->function;
    if (arg_count != function->arity) {
        runtime_error("Expected {} arguments but got {}.", function->arity,
                      arg_count);
        return false;
    }

    if (frames.size() == FRAMES_MAX) {
        runtime_error("Stack overflow.");
        return false;
    }
    frames.emplace_back(closure, function->chunk.code.begin(),
                        stack.end() - arg_count - 1 - stack.begin());
    return true;
}

bool VM::call_native(heap_ptr<ObjNative> native_fn, int arg_count) {
    if (arg_count != native_fn->arity) {
        runtime_error("Expected {} arguments but got {}.", native_fn->arity,
                      arg_count);
        return false;
    }
    using span_size = std::span<Value>::size_type;
    Value result = std::invoke(
        native_fn->fun, std::span<Value>{&stack.back() - arg_count,
                                         static_cast<span_size>(arg_count)});
    // The -1 is to pop the function call itself.
    stack.resize(stack.size() - (arg_count + 1));
    push(result);
    return true;
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

CallFrame::CallFrame(heap_ptr<ObjClosure> closure, CodeVec::const_iterator ip,
                     size_t slots)
    : closure(closure), ip(std::move(ip)), slots(slots) {}

Chunk &CallFrame::chunk() { return closure->function->chunk; }
