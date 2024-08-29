#pragma once

#include "src/syntactics/scanner.h"
#include "src/vm/chunk.h"

struct Compiler {
    Compiler(const std::string &source);

    Chunk compile();

    bool had_error() const;

  private:
    bool m_had_error;
    Scanner scanner;
};