#include "obj_upvalue.h"

ObjUpvalue::ObjUpvalue(size_t location)
    : index(location), closed(std::nullopt) {}

Value &ObjUpvalue::get(std::vector<Value> &stack) {
    if (closed) {
        return closed.value();
    } else {
        if (index >= stack.size()) {
            std::cout << "???????? ";
        }
        return stack[index];
    }
}

const Value &ObjUpvalue::get(const std::vector<Value> &stack) const {
    if (closed) {
        return closed.value();
    } else {
        return stack[index];
    }
}
