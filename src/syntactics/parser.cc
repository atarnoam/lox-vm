#include "parser.h"

Parser::Parser(const std::string &source)
    : current(TokenType::ERROR, "current_start", 0),
      previous(TokenType::ERROR, "previous_start", 0), scanner(source),
      m_had_error(false), m_panic_mode(false) {}

void Parser::advance() {
    previous = current;
    for (;;) {
        current = scanner.scan_token();
        if (current.type != TokenType::ERROR) {
            break;
        }
        error_at_current(current.lexeme);
    }
}

void Parser::consume(TokenType type, const std::string &message) {
    if (current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::check(TokenType type) { return current.type == type; }

bool Parser::had_error() const { return m_had_error; }

bool Parser::panic_mode() const { return m_panic_mode; }

void Parser::error(const std::string_view &message) {
    error_at(previous, message);
}

void Parser::error_at_current(const std::string_view &message) {
    error_at(current, message);
}

void Parser::synchronize() {
    m_panic_mode = false;

    while (current.type != TokenType::END_OF_FILE) {
        if (previous.type == TokenType::SEMICOLON)
            return;
        switch (current.type) {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        default:; // Do nothing.
        }

        advance();
    }
}

void Parser::error_at(const Token &token, const std::string_view &message) {
    if (m_panic_mode) {
        return;
    }
    report_token_error(token, message);
    m_panic_mode = true;
    m_had_error = true;
}
