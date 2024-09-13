#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "src/syntactics/token.h"

struct Scanner {
    explicit Scanner(const std::string &is);

    Token scan_token();

    bool is_at_end() const;

  private:
    std::string_view current_lexeme() const;
    Token make_token(TokenType type) const;
    /** This is char* and not std::string because the `Token`'s string_view will
     * point to it, so we need it to be a constant global.
     */
    Token error_token(const char *message) const;

    void skip_whitespace();

    char peek();
    char peek_next();
    char advance();
    bool match(char expected);

    Token string();
    Token number();
    Token identifier();

    TokenType identifier_type();

    std::string_view source;
    std::string_view::const_iterator token_start;
    std::string_view::const_iterator current;
    size_t line;
};