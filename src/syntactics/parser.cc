#include "parser.h"

Parser::Parser(const std::string &source)
    : current(TokenType::ERROR, "current_start", 0),
      previous(TokenType::ERROR, "previous_start", 0), scanner(source),
      m_had_error(false), panic_mode(false) {}

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

void Parser::consume(TokenType type, const std::string_view &message) {
    if (current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

bool Parser::had_error() const { return m_had_error; }

void Parser::error(const std::string_view &message) {
    error_at(previous, message);
}

void Parser::error_at_current(const std::string_view &message) {
    error_at(current, message);
}

void Parser::error_at(const Token &token, const std::string_view &message) {
    if (panic_mode) {
        return;
    }
    report_token_error(token, message);
    panic_mode = true;
    m_had_error = true;
}
