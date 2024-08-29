#include "compiler.h"

#include <fmt/format.h>

Compiler::Compiler(const std::string &source)
    : m_had_error(false), scanner(source) {}

Chunk Compiler::compile() {
    int curr_line = -1;
    for (;;) {
        Token token = scanner.scan_token();
        if (token.line != curr_line) {
            std::cout << fmt::format("{:4d} ", token.line);
            curr_line = token.line;
        } else {
            std::cout << "   | ";
        }
        std::cout << fmt::format("{:2d} '{}'\n", static_cast<int>(token.type),
                                 token.lexeme);

        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }

    return Chunk();
}

bool Compiler::had_error() const { return m_had_error; }
