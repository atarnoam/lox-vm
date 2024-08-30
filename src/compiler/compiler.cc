#include "compiler.h"

#include <fmt/format.h>

Compiler::Compiler(const std::string &source)
    : m_had_error(false), parser(source) {}

std::optional<Chunk> Compiler::compile() {
    parser.advance();
    expression();
    parser.consume(TokenType::END_OF_FILE, "Expect end of expression.");
    return Chunk();
}

void Compiler::expression() {}

bool Compiler::had_error() const { return m_had_error; }
