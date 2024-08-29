#include "src/syntactics/scanner.h"

#include "scanner.h"
#include <iostream>
#include <sstream>
#include <unordered_map>

const std::unordered_map<std::string, TokenType> keywords{
    {"and", TokenType::AND},       {"class", TokenType::CLASS},
    {"else", TokenType::ELSE},     {"false", TokenType::FALSE},
    {"for", TokenType::FOR},       {"fun", TokenType::FUN},
    {"if", TokenType::IF},         {"nil", TokenType::NIL},
    {"or", TokenType::OR},         {"print", TokenType::PRINT},
    {"return", TokenType::RETURN}, {"super", TokenType::SUPER},
    {"this", TokenType::THIS},     {"true", TokenType::TRUE},
    {"var", TokenType::VAR},       {"while", TokenType::WHILE},
};

bool is_identifier_start(char c) { return isalpha(c) || c == '_'; }
bool is_identifier(char c) { return isalnum(c) || c == '_'; }

Scanner::Scanner(const std::string &is)
    : source(is), token_start(source.begin()), current(source.begin()),
      line(1) {}

Token Scanner::scan_token() {
    skip_whitespace();
    token_start = current;
    char c = advance();

    if (isdigit(c)) {
        return number();
    }
    if (is_identifier_start(c)) {
        return identifier();
    }

    switch (c) {
    case '(':
        return make_token(TokenType::LEFT_PAREN);
    case ')':
        return make_token(TokenType::RIGHT_PAREN);
    case '{':
        return make_token(TokenType::LEFT_BRACE);
    case '}':
        return make_token(TokenType::RIGHT_BRACE);
    case ';':
        return make_token(TokenType::SEMICOLON);
    case ',':
        return make_token(TokenType::COMMA);
    case '.':
        return make_token(TokenType::DOT);
    case '-':
        return make_token(TokenType::MINUS);
    case '+':
        return make_token(TokenType::PLUS);
    case '/':
        return make_token(TokenType::SLASH);
    case '*':
        return make_token(TokenType::STAR);
    case '!':
        return make_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    case '=':
        return make_token(match('=') ? TokenType::EQUAL_EQUAL
                                     : TokenType::EQUAL);
    case '<':
        return make_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    case '>':
        return make_token(match('=') ? TokenType::GREATER_EQUAL
                                     : TokenType::GREATER);
    case '"':
        return string();
    default:
        break;
    }

    if (is_at_end()) {
        return make_token(TokenType::END_OF_FILE);
    }
    return error_token("Unexpected character.");
}

bool Scanner::is_at_end() const { return current >= source.end(); }

std::string_view Scanner::current_lexeme() const {
    return std::string_view(token_start, current);
}

Token Scanner::make_token(TokenType type) const {
    return Token(type, current_lexeme(), line);
}

Token Scanner::error_token(const std::string &message) const {
    return Token(TokenType::ERROR, message, line);
}

void Scanner::skip_whitespace() {
    while (!is_at_end()) {
        char c = peek();
        switch (c) {
        case '\n':
            line++;
            // fallthrough
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '/':
            if (peek_next() == '/') {
                while (peek() != '\n' and !is_at_end()) {
                    advance();
                }
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

char Scanner::peek() { return *current; }

char Scanner::peek_next() {
    if (current + 1 >= source.end()) {
        return '\0';
    }
    return *(current + 1);
}

char Scanner::advance() { return *(current++); }

bool Scanner::match(char expected) {
    if (is_at_end())
        return false;
    if (*current != expected)
        return false;
    current++;
    return true;
}

Token Scanner::string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (is_at_end())
        return error_token("Unterminated string.");

    // The closing quote.
    advance();
    return make_token(TokenType::STRING);
}

Token Scanner::number() {
    while (isdigit(peek()))
        advance();

    // Look for a fractional part.
    if (peek() == '.' and isdigit(peek_next())) {
        // Consume the "."
        advance();

        while (isdigit(peek()))
            advance();
    }

    return make_token(TokenType::NUMBER);
}

Token Scanner::identifier() {
    while (is_identifier(peek())) {
        advance();
    }

    auto type_it = keywords.find(std::string(current_lexeme()));
    TokenType type =
        type_it == keywords.end() ? TokenType::IDENTIFIER : type_it->second;
    return make_token(type);
}

TokenType Scanner::identifier_type() { return TokenType::IDENTIFIER; }
