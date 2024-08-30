#pragma once

#include <string>
#include <string_view>

#include "src/syntactics/token.h"

void report_token_error(const Token &token, const std::string &message,
                        std::ostream &os = std::cerr);
void report_token_error(const Token &token, const std::string_view &message,
                        std::ostream &os = std::cerr);