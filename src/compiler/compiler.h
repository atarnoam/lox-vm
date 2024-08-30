#pragma once

#include "src/syntactics/parser.h"
#include "src/vm/chunk.h"

#include <optional>

struct Compiler {
    Compiler(const std::string &source);

    std::optional<Chunk> compile();

    void expression();

    bool had_error() const;

  private:
    bool m_had_error;
    Parser parser;
};