#include "obj_function.h"

ObjFunction::ObjFunction(int arity, heap_ptr<ObjString> name)
    : arity(arity), name(name), upvalue_count(0), chunk() {}

ObjClosure::ObjClosure(heap_ptr<ObjFunction> function)
    : function(function), upvalue_count(function->upvalue_count) {
    upvalues.reserve(upvalue_count);
}
