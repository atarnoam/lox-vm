#include "compiler.h"

#include "src/debug_flags.h"
#include "src/vm/debug.h"
#include <array>
#include <charconv>
#include <fmt/format.h>
#include <functional>
#include <limits>

extern constinit ParseRule rules[];

Precedence operator+(Precedence precedence, int other) {
    return static_cast<Precedence>(static_cast<int>(precedence) + other);
}

Compiler::Compiler(const std::string &source)
    : parser(source), compiling_chunk() {}

std::optional<Chunk> Compiler::compile() {
    parser.advance();
    expression();
    parser.consume(TokenType::END_OF_FILE, "Expect end of expression.");
    end_compilation();

    if (parser.had_error()) {
        return std::nullopt;
    }
    return compiling_chunk;
}

void Compiler::expression() { parse_precedence(Precedence::ASSIGNMENT); }

void Compiler::number() {
    double value;
    std::string_view &lexeme = parser.previous.lexeme;
    std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), value);
    emit_constant(value);
}

void Compiler::grouping() {
    expression();
    parser.consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary() {
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

void Compiler::binary() {
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

void Compiler::literal() {
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

Chunk &Compiler::current_chunk() { return compiling_chunk; }

void Compiler::end_compilation() {
    emit_return();
    if constexpr (DEBUG_PRINT_CODE) {
        if (!parser.had_error()) {
            disassemble_chunk(current_chunk(), "code");
        }
    }
}

void Compiler::emit(InstructionData data) {
    current_chunk().write(data, parser.previous.line);
}

void Compiler::emit(InstructionData data1, InstructionData data2) {
    emit(data1);
    emit(data2);
}

void Compiler::emit_return() { emit(OpCode::RETURN); }

void Compiler::emit_constant(Value value) {
    emit(OpCode::CONSTANT, make_constant(value));
}

ConstRefT Compiler::make_constant(Value value) {
    int constant_ref = current_chunk().add_constant(value);
    if (constant_ref > std::numeric_limits<ConstRefT>::max()) {
        // Not really a parser error, but we use the parser for bookkeeping
        // errors.
        parser.error("Too many constants in one chunk.");
        return 0;
    }
    return constant_ref;
}

void Compiler::parse_precedence(Precedence precedence) {
    parser.advance();
    ParseFn prefix_rule = get_rule(parser.previous.type).prefix;
    if (prefix_rule == nullptr) {
        parser.error("Expect expression.");
        return;
    }

    std::invoke(prefix_rule, this);

    while (precedence <= get_rule(parser.current.type).precedence) {
        parser.advance();
        ParseFn infix_rule = get_rule(parser.previous.type).infix;
        std::invoke(infix_rule, this);
    }
}

ParseRule &get_rule(TokenType token_type) {
    return rules[static_cast<int>(token_type)];
}

constinit ParseRule rules[]{
#define TOKEN(x) static_cast<int>((TokenType::x))
#define FUNC(f) &Compiler::f
    [TOKEN(LEFT_PAREN)] = {FUNC(grouping), nullptr, Precedence::NONE},
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
    [TOKEN(IDENTIFIER)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(STRING)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(NUMBER)] = {FUNC(number), nullptr, Precedence::NONE},
    [TOKEN(AND)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(CLASS)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(ELSE)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(FALSE)] = {FUNC(literal), nullptr, Precedence::NONE},
    [TOKEN(FOR)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(FUN)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(IF)] = {nullptr, nullptr, Precedence::NONE},
    [TOKEN(NIL)] = {FUNC(literal), nullptr, Precedence::NONE},
    [TOKEN(OR)] = {nullptr, nullptr, Precedence::NONE},
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
