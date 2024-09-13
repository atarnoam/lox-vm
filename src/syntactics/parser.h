#pragma once

#include "src/syntactics/logging.h"
#include "src/syntactics/scanner.h"
#include "src/syntactics/token.h"

/**
 * A simple "parser" that has two functionalities:
 * 1. Hold the current and last token, for use of the compiler.
 * 2. Wrap error tokens so the compiler doesn't need to handle them.
 */
struct Parser {
    Parser(const std::string &source);

    Token current;
    Token previous;

    void advance();
    void consume(TokenType type, const std::string &message);
    bool match(TokenType type);
    bool check(TokenType type);

    bool had_error() const;
    bool panic_mode() const;

    void error(const std::string_view &message);
    void error_at_current(const std::string_view &message);

    void synchronize();

  private:
    void error_at(const Token &token, const std::string_view &message);

    Scanner scanner;

    bool m_had_error;
    bool m_panic_mode;
};
