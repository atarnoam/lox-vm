#include "obj_upvalue.h"

ObjUpvalue::ObjUpvalue(size_t location)
    : index(location), value(std::nullopt) {}

Value &ObjUpvalue::get(std::vector<Value> &stack) {
    if (value) {
        return value.value();
    } else {
        return stack[index];
    }
}

const Value &ObjUpvalue::get(const std::vector<Value> &stack) const {
    if (value) {
        return value.value();
    } else {
        return stack[index];
    }
}
