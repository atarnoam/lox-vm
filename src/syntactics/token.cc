#include "token.h"

#include <sstream>

Token::Token(TokenType type, std::string_view lexeme, int line)
    : type(type), lexeme(std::move(lexeme)), line(line) {}
