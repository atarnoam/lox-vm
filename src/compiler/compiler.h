#pragma once

#include "src/syntactics/parser.h"
#include "src/vm/chunk.h"
#include "src/vm/heap_manager.h"
#include "src/vm/value.h"

#include <optional>

struct Compiler;
typedef void (Compiler::*ParseFn)(bool);

enum struct Precedence {
    NONE = 0,
    ASSIGNMENT, // =
    OR,         // or
    AND,        // and
    EQUALITY,   // == !=
    COMPARISON, // < > <= >=
    TERM,       // + -
    FACTOR,     // * /
    UNARY,      // ! -
    CALL,       // . ()
    PRIMARY
};

Precedence operator+(Precedence precedence, int other);

struct Compiler {
    Compiler(HeapManager &heap_manager, const std::string &source);

    std::optional<Chunk> compile();

    void expression();
    void statement();
    void declaration();

    // Statements
    void print_statement();
    void expression_statement();
    void var_declaration();

    // Expressions
    void number(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void binary(bool can_assign);
    void literal(bool can_assign);
    void string(bool can_assign);
    void variable(bool can_assign);

  private:
    const_ref_t parse_variable(const std::string &error_message);
    void define_variable(const_ref_t global);

    Chunk &current_chunk();

    void end_compilation();

    void emit(InstructionData data);
    void emit(InstructionData data1, InstructionData data2);

    void emit_return();
    void emit_constant(Value value);

    const_ref_t make_constant(Value value);
    const_ref_t identifier_constant(const Token &name);

    void named_variable(const Token &name, bool can_assign);

    void parse_precedence(Precedence precedence);

    HeapManager &heap_manager;
    Parser parser;
    // Change to unique_ptr?
    Chunk compiling_chunk;
};

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

ParseRule &get_rule(TokenType token_type);