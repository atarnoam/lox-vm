#include "obj_upvalue.h"

ObjUpvalue::ObjUpvalue(size_t location)
    : index(location), closed(std::nullopt) {}
