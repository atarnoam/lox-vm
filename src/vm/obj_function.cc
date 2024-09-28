#include "obj_function.h"

ObjFunction::ObjFunction(int arity, heap_ptr<ObjString> name)
    : arity(arity), name(name), upvalue_count(0), chunk() {}
