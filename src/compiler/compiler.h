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

/** A local variable. */
struct Local {
    Local(Token name);
    Local(Token name, int depth);

    void mark_initialized(int depth);
    bool is_initialized() const;

    Token name;
    // depth = -1 means the variable is not initialized.
    int depth;
};

struct Compiler {
    Compiler(HeapManager &heap_manager, const std::string &source);

    std::optional<Chunk> compile();

    // Expressions. These are public for the `rules` table.
    void number(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void binary(bool can_assign);
    void literal(bool can_assign);
    void string(bool can_assign);
    void variable(bool can_assign);

  private:
    void expression();
    void block();
    void statement();
    void declaration();

    // Statements
    void print_statement();
    void expression_statement();
    void var_declaration();

    void begin_scope();
    void end_scope();

    void declare_variable();
    const_ref_t parse_variable(const std::string &error_message);
    void define_variable(const_ref_t global);
    void add_local(const Token &name);

    Chunk &current_chunk();

    void end_compilation();

    void emit(InstructionData data);
    void emit(InstructionData data1, InstructionData data2);

    void emit_return();
    void emit_constant(const Value &value);

    const_ref_t make_constant(const Value &value);
    const_ref_t identifier_constant(const Token &name);
    std::optional<const_ref_t> resolve_local(const Token &name);

    void named_variable(const Token &name, bool can_assign);

    void parse_precedence(Precedence precedence);

    HeapManager &heap_manager;
    Parser parser;
    // Change to unique_ptr?
    Chunk compiling_chunk;
    std::vector<Local> locals;
    int scope_depth;
};

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

ParseRule &get_rule(TokenType token_type);