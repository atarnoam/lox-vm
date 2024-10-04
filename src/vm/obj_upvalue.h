#pragma once

#include "src/vm/obj_upvalue.fwd.h"
#include "src/vm/value.h"

#include <optional>

struct ObjUpvalue {
    ObjUpvalue(size_t location);

    template <typename Allocator>
    Value &get(std::vector<Value, Allocator> &stack) {
        if (closed) {
            return closed.value();
        } else {
            return stack[index];
        }
    }
    template <typename Allocator>
    const Value &get(const std::vector<Value, Allocator> &stack) const {
        if (closed) {
            return closed.value();
        } else {
            return stack[index];
        }
    }

    // Index of local variable captured by the upvalue, relative to the frame
    // start.
    size_t index;
    std::optional<Value> closed;
};