#include "compiler.h"

#include "src/debug_flags.h"
#include "src/vm/debug.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <fmt/format.h>
#include <functional>
#include <limits>

extern constinit ParseRule rules[];

constexpr auto CONST_REF_T_MAX = std::numeric_limits<const_ref_t>::max();
constexpr auto JUMP_OFF_T_MAX = std::numeric_limits<jump_off_t>::max();

Precedence operator+(Precedence precedence, int other) {
    return static_cast<Precedence>(static_cast<int>(precedence) + other);
}

Token empty_token() { return {TokenType::END_OF_FILE, "", 0}; }

Local empty_local() { return {empty_token(), 0}; }

Compiler::Compiler(HeapManager &heap_manager, Parser &parser, FunctionType type,
                   Compiler *enclosing)
    : heap_manager(heap_manager), parser(parser), locals({empty_local()}),
      scope_depth(0), compiling_function(type == FunctionType::SCRIPT
                                             ? heap_manager.new_function()
                                             : heap_manager.new_function(
                                                   parser.previous.lexeme)),
      type(type), enclosing(enclosing), upvalues() {}

std::optional<heap_ptr<ObjFunction>> Compiler::compile() {
    parser.advance();
    while (!parser.match(TokenType::END_OF_FILE)) {
        declaration();

        if (parser.panic_mode()) {
            parser.synchronize();
        }
    }
    end_compilation();

    if (parser.had_error()) {
        return std::nullopt;
    }
    return compiling_function;
}

void Compiler::expression() { parse_precedence(Precedence::ASSIGNMENT); }

void Compiler::block() {
    while (!parser.check(TokenType::RIGHT_BRACE) and
           !parser.check(TokenType::END_OF_FILE)) {
        declaration();
    }

    parser.consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::statement() {
    if (parser.match(TokenType::PRINT)) {
        print_statement();
    } else if (parser.match(TokenType::IF)) {
        if_statement();
    } else if (parser.match(TokenType::RETURN)) {
        return_statement();
    } else if (parser.match(TokenType::WHILE)) {
        while_statement();
    } else if (parser.match(TokenType::FOR)) {
        for_statement();
    } else if (parser.match(TokenType::LEFT_BRACE)) {
        begin_scope();
        block();
        end_scope();
    } else {
        expression_statement();
    }
}

void Compiler::declaration() {
    if (parser.match(TokenType::VAR)) {
        var_declaration();
    } else if (parser.match(TokenType::FUN)) {
        fun_declaration();
    } else {
        statement();
    }
}

void Compiler::function(FunctionType type) {
    static const std::string error_message =
        std::string("Can't have more than ") + std::to_string(CONST_REF_T_MAX) +
        " parameters.";

    Compiler compiler{heap_manager, parser, type, this};
    compiler.begin_scope();

    parser.consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    if (!parser.check(TokenType::RIGHT_PAREN)) {
        do {
            compiler.compiling_function->arity++;
            if (compiler.compiling_function->arity > CONST_REF_T_MAX) {
                parser.error_at_current(error_message);
            }
            const_ref_t constant =
                compiler.parse_variable("Expect parameter name.");
            compiler.define_variable(constant);
        } while (parser.match(TokenType::COMMA));
    }
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    parser.consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");

    compiler.block();

    compiler.end_scope();
    auto function = compiler.end_compilation();
    // TODO: If closure is not needed, emit function.
    emit(OpCode::CLOSURE, make_constant(function));
    for (const auto &upvalue : compiler.upvalues) {
        emit(upvalue.is_local ? 1 : 0, upvalue.index);
    }
}

void Compiler::print_statement() {
    expression();
    parser.consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emit(OpCode::PRINT);
}

void Compiler::expression_statement() {
    expression();
    parser.consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    emit(OpCode::POP);
}

void Compiler::if_statement() {
    parser.consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int then_jump = emit_jump(OpCode::JUMP_IF_FALSE);
    emit(OpCode::POP);
    statement();

    int else_jump = emit_jump(OpCode::JUMP);
    patch_jump(then_jump);
    emit(OpCode::POP);

    if (parser.match(TokenType::ELSE)) {
        statement();
    }
    patch_jump(else_jump);
}

void Compiler::while_statement() {
    int loop_start = current_chunk().code.size();

    parser.consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int exit_jump = emit_jump(OpCode::JUMP_IF_FALSE);
    emit(OpCode::POP);
    statement();
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit(OpCode::POP);
}

void Compiler::for_statement() {
    begin_scope();
    parser.consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    if (parser.match(TokenType::SEMICOLON)) {
        // No initializer.
    } else if (parser.match(TokenType::VAR)) {
        var_declaration();
    } else {
        expression_statement();
    }
    int loop_start = current_chunk().code.size();
    // TODO: change to optional?
    int exit_jump = -1;
    if (!parser.match(TokenType::SEMICOLON)) {
        expression();
        parser.consume(TokenType::SEMICOLON,
                       "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exit_jump = emit_jump(OpCode::JUMP_IF_FALSE);
        emit(OpCode::POP); // Condition.
    }

    if (!parser.match(TokenType::RIGHT_PAREN)) {
        int body_jump = emit_jump(OpCode::JUMP);
        int incrementStart = current_chunk().code.size();
        expression();
        emit(OpCode::POP);
        parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

        emit_loop(loop_start);
        loop_start = incrementStart;
        patch_jump(body_jump);
    }

    statement();
    emit_loop(loop_start);

    if (exit_jump != -1) {
        patch_jump(exit_jump);
        emit(OpCode::POP); // Condition.
    }

    end_scope();
}

void Compiler::return_statement() {
    if (type == FunctionType::SCRIPT) {
        parser.error("Can't return from top-level code.");
    }

    if (parser.match(TokenType::SEMICOLON)) {
        emit_return();
    } else {
        expression();
        parser.consume(TokenType::SEMICOLON, "Expect ';' after return value.");
        emit(OpCode::RETURN);
    }
}

void Compiler::var_declaration() {
    const_ref_t global = parse_variable("Expect variable name.");

    if (parser.match(TokenType::EQUAL)) {
        expression();
    } else {
        emit(OpCode::NIL);
    }
    parser.consume(TokenType::SEMICOLON,
                   "Expect ';' after variable declaration.");

    define_variable(global);
}

void Compiler::fun_declaration() {
    const_ref_t global = parse_variable("Expect function name.");
    // Functions can be used
    mark_initialized_last();

    function(FunctionType::FUNCTION);
    define_variable(global);
}

void Compiler::begin_scope() { scope_depth++; }

void Compiler::end_scope() {
    scope_depth--;

    // TODO: add OpCode::POPN to pop many times.
    while (locals.size() > 0 and locals.back().depth > scope_depth) {
        if (locals.back().is_captured) {
            emit(OpCode::CLOSE_UPVALUE);
        } else {
            emit(OpCode::POP);
        }
        locals.pop_back();
    }
}

void Compiler::number(bool can_assign) {
    double value;
    std::string_view &lexeme = parser.previous.lexeme;
    std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), value);
    emit_constant(value);
}

void Compiler::grouping(bool can_assign) {
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(bool can_assign) {
    TokenType operator_type = parser.previous.type;

    // Compile operand.
    parse_precedence(Precedence::UNARY);

    // Emit operator instruction.
    switch (operator_type) {
    case TokenType::MINUS:
        emit(OpCode::NEGATE);
        break;
    case TokenType::BANG:
        emit(OpCode::NOT);
        break;
    default:
        return; // Unreachable.
    }
}

void Compiler::binary(bool can_assign) {
    TokenType operator_type = parser.previous.type;
    ParseRule &rule = get_rule(operator_type);
    parse_precedence(rule.precedence + 1);

    switch (operator_type) {
    case TokenType::BANG_EQUAL:
        emit(OpCode::EQUAL, OpCode::NOT);
        break;
    case TokenType::EQUAL_EQUAL:
        emit(OpCode::EQUAL);
        break;
    case TokenType::GREATER:
        emit(OpCode::GREATER);
        break;
    case TokenType::GREATER_EQUAL:
        emit(OpCode::LESS, OpCode::NOT);
        break;
    case TokenType::LESS:
        emit(OpCode::LESS);
        break;
    case TokenType::LESS_EQUAL:
        emit(OpCode::GREATER, OpCode::NOT);
        break;

    case TokenType::PLUS:
        emit(OpCode::ADD);
        break;
    case TokenType::MINUS:
        emit(OpCode::SUBTRACT);
        break;
    case TokenType::STAR:
        emit(OpCode::MULTIPLY);
        break;
    case TokenType::SLASH:
        emit(OpCode::DIVIDE);
        break;
    default:
        return; // Unreachable.
    }
}

void Compiler::literal(bool can_assign) {
    switch (parser.previous.type) {
    case TokenType::FALSE:
        emit(OpCode::FALSE);
        break;
    case TokenType::NIL:
        emit(OpCode::NIL);
        break;
    case TokenType::TRUE:
        emit(OpCode::TRUE);
        break;
    default:
        return; // Unreachable
    }
}

void Compiler::string(bool can_assign) {
    auto lexeme = parser.previous.lexeme;
    emit_constant(heap_manager.initialize(lexeme.substr(1, lexeme.size() - 2)));
}

void Compiler::variable(bool can_assign) {
    named_variable(parser.previous, can_assign);
}

void Compiler::and_(bool can_assign) {
    int end_jump = emit_jump(OpCode::JUMP_IF_FALSE);
    emit(OpCode::POP);
    parse_precedence(Precedence::AND);

    patch_jump(end_jump);
}

void Compiler::or_(bool can_assign) {
    int end_jump = emit_jump(OpCode::JUMP_IF_TRUE);
    emit(OpCode::POP);
    parse_precedence(Precedence::OR);

    patch_jump(end_jump);
}

void Compiler::call(bool can_assign) {
    const_ref_t arg_count = argument_list();
    emit(OpCode::CALL, arg_count);
}

void Compiler::declare_variable() {
    Token name = parser.previous;

    if (scope_depth == 0) {
        return;
    }

    // Declare local variable.
    for (auto local_it = locals.rbegin(); local_it < locals.rend();
         local_it++) {
        if (local_it->is_initialized() and local_it->depth < scope_depth) {
            break;
        }

        if (name.lexeme == local_it->name.lexeme) {
            parser.error("Already a variable with this name in this scope.");
        }
    }
    add_local(name);
}

const_ref_t Compiler::parse_variable(const std::string &error_message) {
    parser.consume(TokenType::IDENTIFIER, error_message);

    declare_variable();
    if (scope_depth > 0) {
        return 0;
    }

    return identifier_constant(parser.previous);
}

void Compiler::define_variable(const_ref_t global) {
    if (scope_depth > 0) {
        mark_initialized_last();
        // Local variables live on the stack.
        return;
    }
    emit(OpCode::DEFINE_GLOBAL, global);
}

void Compiler::add_local(const Token &name) {
    if (locals.size() >= CONST_REF_T_MAX) {
        parser.error("Too many local variables in function.");
        return;
    }

    locals.emplace_back(name, -1);
}

Chunk &Compiler::current_chunk() { return compiling_function->chunk; }

heap_ptr<ObjFunction> Compiler::end_compilation() {
    emit(OpCode::NIL);
    emit_return();
    if constexpr (DEBUG_PRINT_CODE) {
        if (!parser.had_error()) {
            disassemble_chunk(current_chunk(),
                              compiling_function->name !=
                                      heap_ptr<ObjString>(nullptr)
                                  ? compiling_function->name->str()
                                  : "<script>");
        }
    }

    return compiling_function;
}

void Compiler::emit_return() { emit(OpCode::RETURN); }

void Compiler::emit_constant(const Value &value) {
    emit(OpCode::CONSTANT, make_constant(value));
}

int Compiler::emit_jump(OpCode instruction) {
    emit(instruction);
    // Empty for now, to be patched
    return emit_jump_value();
}

int Compiler::emit_jump_value(jump_off_t value) {
    int jump_offset = emit_jump_value();
    // We might be able to write it without initializing-then-setting, but
    // whatever.
    current_chunk().jump_at(jump_offset) = value;
    return jump_offset;
}

int Compiler::emit_jump_value() {
    int offset = current_chunk().code.size();
    for (size_t i = 0; i < sizeof(jump_off_t); ++i) {
        emit(0xff);
    }
    return offset;
}

void Compiler::emit_loop(int loop_start) {
    emit(OpCode::LOOP);
    int offset = current_chunk().code.size() - loop_start + sizeof(jump_off_t);
    if (offset > JUMP_OFF_T_MAX) {
        parser.error("Loop body too large.");
    }

    emit_jump_value(offset);
}

const_ref_t Compiler::make_constant(const Value &value) {
    int constant_ref = current_chunk().add_constant(value);
    if (constant_ref > CONST_REF_T_MAX) {
        // Not really a parser error, but we use the parser for bookkeeping
        // errors.
        parser.error("Too many constants in one chunk.");
        return 0;
    }
    return constant_ref;
}

const_ref_t Compiler::identifier_constant(const Token &name) {
    return make_constant(heap_manager.initialize(name.lexeme));
}

std::optional<const_ref_t> Compiler::resolve_local(const Token &name) {
    auto local = std::find_if(locals.rbegin(), locals.rend(),
                              [&name](const auto &local) {
                                  return local.name.lexeme == name.lexeme;
                              });

    if (local == locals.rend()) {
        return std::nullopt;
    }
    if (!local->is_initialized()) {
        parser.error("Can't read local variable in its own initializer.");
    }
    return static_cast<const_ref_t>(std::distance(local, locals.rend()) - 1);
}

std::optional<const_ref_t> Compiler::resolve_upvalue(const Token &name) {
    if (enclosing == nullptr) {
        return std::nullopt;
    }

    auto local = enclosing->resolve_local(name);
    if (local) {
        enclosing->locals[local.value()].is_captured = true;
        return add_upvalue(local.value(), true);
    }

    auto upvalue = enclosing->resolve_upvalue(name);
    if (upvalue) {
        return add_upvalue(upvalue.value(), false);
    }

    return std::nullopt;
}

const_ref_t Compiler::add_upvalue(const_ref_t index, bool is_local) {
    Upvalue new_upvalue{index, is_local};
    auto upvalue_it = std::ranges::find(upvalues, new_upvalue);
    if (upvalue_it != upvalues.end()) {
        return static_cast<const_ref_t>(upvalue_it - upvalues.begin());
    }

    if (compiling_function->upvalue_count == CONST_REF_T_MAX) {
        parser.error("Too many closure variables in function.");
        return 0;
    }

    upvalues.push_back(new_upvalue);
    return compiling_function->upvalue_count++;
}

void Compiler::named_variable(const Token &name, bool can_assign) {
    OpCode get_op, set_op;
    const_ref_t arg;

    std::optional<const_ref_t> arg_opt = resolve_local(name);
    if (arg_opt) {
        arg = arg_opt.value();
        get_op = OpCode::GET_LOCAL;
        set_op = OpCode::SET_LOCAL;
    } else if ((arg_opt = resolve_upvalue(name))) {
        arg = arg_opt.value();
        get_op = OpCode::GET_UPVALUE;
        set_op = OpCode::SET_UPVALUE;
    } else {
        arg = identifier_constant(name);
        get_op = OpCode::GET_GLOBAL;
        set_op = OpCode::SET_GLOBAL;
    }

    if (can_assign and parser.match(TokenType::EQUAL)) {
        expression();
        emit(set_op, arg);
    } else {
        emit(get_op, arg);
    }
}

void Compiler::mark_initialized_last() {
    if (scope_depth == 0)
        return;
    locals.back().mark_initialized(scope_depth);
}

size_t Compiler::argument_list() {
    size_t arg_count = 0;

    const static std::string error_message =
        std::string("Can't have more than ") + std::to_string(CONST_REF_T_MAX) +
        " arguments.";

    if (!parser.check(TokenType::RIGHT_PAREN)) {
        do {
            expression();
            if (arg_count >= CONST_REF_T_MAX) {
                parser.error(error_message);
            }
            arg_count++;
        } while (parser.match(TokenType::COMMA));
    }
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return arg_count;
}

void Compiler::patch_jump(int offset) {
    // the minus to adjust for the bytecode for the jump offset itself.
    int jump = current_chunk().code.size() - offset - sizeof(jump_off_t);

    if (jump > JUMP_OFF_T_MAX) {
        parser.error("Too much code to jump over.");
    }

    current_chunk().jump_at(offset) = jump;
}

void Compiler::parse_precedence(Precedence precedence) {
    parser.advance();

    ParseFn prefix_rule = get_rule(parser.previous.type).prefix;
    if (prefix_rule == nullptr) {
        parser.error("Expect expression.");
        return;
    }

    bool can_assign = precedence <= Precedence::ASSIGNMENT;
    std::invoke(prefix_rule, this, can_assign);

    while (precedence <= get_rule(parser.current.type).precedence) {
        parser.advance();
        ParseFn infix_rule = get_rule(parser.previous.type).infix;
        std::invoke(infix_rule, this, can_assign);
    }

    if (can_assign && parser.match(TokenType::EQUAL)) {
        parser.error("Invalid assignment target.");
    }
}

ParseRule &get_rule(TokenType token_type) {
    return rules[static_cast<int>(token_type)];
}

constinit ParseRule rules[]{
#define TOKEN(x) static_cast<int>((TokenType::x))
#define FUNC(f) &Compiler::f
    [TOKEN(LEFT_PAREN)] = {FUNC(grouping), FUNC(call), Precedence::CALL},
    [TOKEN(RIGHT_PAREN)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(LEFT_BRACE)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(RIGHT_BRACE)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(COMMA)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(DOT)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(MINUS)] = {FUNC(unary), FUNC(binary), Precedence::TERM},
    [TOKEN(PLUS)] = {nullptr, FUNC(binary), Precedence::TERM},
    [TOKEN(SEMICOLON)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(SLASH)] = {nullptr, FUNC(binary), Precedence::FACTOR},
    [TOKEN(STAR)] = {nullptr, FUNC(binary), Precedence::FACTOR},
    [TOKEN(BANG)] = {FUNC(unary), nullptr, Precedence::NONE},
    [TOKEN(BANG_EQUAL)] = {nullptr, FUNC(binary), Precedence::EQUALITY},
    [TOKEN(EQUAL)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(EQUAL_EQUAL)] = {nullptr, FUNC(binary), Precedence::EQUALITY},
    [TOKEN(GREATER)] = {nullptr, FUNC(binary), Precedence::COMPARISON},
    [TOKEN(GREATER_EQUAL)] = {nullptr, FUNC(binary), Precedence::COMPARISON},
    [TOKEN(LESS)] = {nullptr, FUNC(binary), Precedence::COMPARISON},
    [TOKEN(LESS_EQUAL)] = {nullptr, FUNC(binary), Precedence::COMPARISON},
    [TOKEN(IDENTIFIER)] = {FUNC(variable), nullptr, Precedence::NONE},
    [TOKEN(STRING)] = {FUNC(string), nullptr, Precedence::NONE},
    [TOKEN(NUMBER)] = {FUNC(number), nullptr, Precedence::NONE},
    [TOKEN(AND)] = {nullptr, FUNC(and_), Precedence::AND},
    [TOKEN(CLASS)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(ELSE)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(FALSE)] = {FUNC(literal), nullptr, Precedence::NONE},
    [TOKEN(FOR)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(FUN)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(IF)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(NIL)] = {FUNC(literal), nullptr, Precedence::NONE},
    [TOKEN(OR)] = {nullptr, FUNC(or_), Precedence::OR},
    [TOKEN(PRINT)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(RETURN)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(SUPER)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(THIS)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(TRUE)] = {FUNC(literal), nullptr, Precedence::NONE},
    [TOKEN(VAR)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(WHILE)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(END_OF_FILE)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(ERROR)] = {nullptr, nullptr, Precedence::NONE},
#undef FUNC
#undef TOKEN
};

Local::Local(Token name) : Local(name, -1) {}

Local::Local(Token name, int depth)
    : name(name), depth(depth), is_captured(false) {}

void Local::mark_initialized(int depth) { this->depth = depth; }

bool Local::is_initialized() const { return depth != -1; }
