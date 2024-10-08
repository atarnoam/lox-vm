#pragma once

#include "src/syntactics/parser.h"
#include "src/vm/chunk.h"
#include "src/vm/heap_manager.h"
#include "src/vm/obj_function.h"
#include "src/vm/value.h"

#include <optional>
#include <type_traits>

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
    bool is_captured;
};

struct Upvalue {
    const_ref_t index;
    bool is_local;

    bool operator==(const Upvalue &other) const = default;
};

enum struct FunctionType { FUNCTION, SCRIPT };

struct Compiler {
    Compiler(HeapManager &heap_manager, Parser &parser,
             FunctionType type = FunctionType::SCRIPT,
             Compiler *enclosing = nullptr);

    std::optional<heap_ptr<ObjFunction>> compile();

    // Expressions. These are public for the `rules` table.
    void number(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void binary(bool can_assign);
    void literal(bool can_assign);
    void string(bool can_assign);
    void variable(bool can_assign);
    void and_(bool can_assign);
    void or_(bool can_assign);
    void call(bool can_assign);

  private:
    void expression();
    void block();
    void statement();
    void declaration();

    void function(FunctionType type);

    // Statements
    void print_statement();
    void expression_statement();
    void if_statement();
    void while_statement();
    void for_statement();
    void return_statement();

    void var_declaration();
    void fun_declaration();

    void begin_scope();
    void end_scope();

    void declare_variable();
    const_ref_t parse_variable(const std::string &error_message);
    void define_variable(const_ref_t global);
    void add_local(const Token &name);

    Chunk &current_chunk();

    heap_ptr<ObjFunction> end_compilation();

    template <typename... Args>
    requires(std::convertible_to<Args, InstructionData> and...) void emit(
        Args... data) {
        ([&]() { current_chunk().write(data, parser.previous.line); }(), ...);
    };

    void emit_return();
    void emit_constant(const Value &value);
    int emit_jump(OpCode instruction);
    /** returns index of jump value. */
    int emit_jump_value(jump_off_t value);
    // Empty jump
    int emit_jump_value();
    void emit_loop(int loop_start);

    const_ref_t make_constant(const Value &value);
    const_ref_t identifier_constant(const Token &name);
    std::optional<const_ref_t> resolve_local(const Token &name);
    std::optional<const_ref_t> resolve_upvalue(const Token &name);

    const_ref_t add_upvalue(const_ref_t index, bool is_local);

    void named_variable(const Token &name, bool can_assign);
    // Mark last variable defined as initialized, with depth the current scope
    // depth.
    void mark_initialized_last();

    size_t argument_list();

    void patch_jump(int offset);

    void parse_precedence(Precedence precedence);

    HeapManager &heap_manager;
    Parser &parser;
    std::vector<Local> locals;
    int scope_depth;
    heap_ptr<ObjFunction> compiling_function;
    FunctionType type;
    Compiler *const enclosing;
    std::vector<Upvalue> upvalues;
};

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

ParseRule &get_rule(TokenType token_type);