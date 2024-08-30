#include "logging.h"

#include <fmt/format.h>
#include <iostream>
#include <string>

void start_token_error(const Token &token, std::ostream &os) {
    os << fmt::format("[line {}] Error", token.line);

    if (token.type == TokenType::END_OF_FILE) {
        os << " at end: ";
    } else if (token.type == TokenType::ERROR) {
        os << ": ";
    } else {
        os << fmt::format(" at '{}': ", token.lexeme);
    }
}

void report_token_error(const Token &token, const std::string &message,
                        std::ostream &os) {
    report_token_error(token, message, os);
}

void report_token_error(const Token &token, const std::string_view &message,
                        std::ostream &os) {
    start_token_error(token, os);
    os << message << std::endl;
}
