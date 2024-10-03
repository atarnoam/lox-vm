#include "obj_upvalue.h"

ObjUpvalue::ObjUpvalue(Value *location)
    : location(location), value(std::nullopt) {}
