#pragma once

#include "src/vm/obj_upvalue.fwd.h"
#include "src/vm/value.h"

#include <optional>

struct ObjUpvalue {
    ObjUpvalue(size_t location);

    Value &get(std::vector<Value> &stack);
    const Value &get(const std::vector<Value> &stack) const;

    // Index of local variable captured by the upvalue, relative to the frame
    // start.
    size_t index;
    std::optional<Value> value;
};