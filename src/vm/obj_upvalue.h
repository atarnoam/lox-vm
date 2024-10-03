#pragma once

#include "src/vm/obj_upvalue.fwd.h"
#include "src/vm/value.h"

#include <optional>

struct ObjUpvalue {
    ObjUpvalue(Value *location);

    Value *location;
    std::optional<Value> value;
};