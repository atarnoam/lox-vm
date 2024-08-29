#include "src/vm/value.h"
#include "value.h"

#include <fmt/format.h>

std::string to_string(Value value) {
    return fmt::format("{:g}", static_cast<double>(value));
}
