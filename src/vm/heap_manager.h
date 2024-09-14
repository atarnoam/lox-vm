#pragma once

#include "src/vm/gc/heap.h"
#include "src/vm/obj_function.h"
#include "src/vm/value.h"

#include <string>
#include <string_view>

struct HeapManager {
    HeapManager();

    Value initialize(auto t) { return Value(t); }

    Value initialize(const std::string &string);
    Value initialize(const std::string_view &string);

    heap_ptr<ObjFunction> new_function(int arity = 0);
    heap_ptr<ObjFunction> new_function(int arity, heap_ptr<ObjString> name);
    heap_ptr<ObjFunction> new_function(int arity, const std::string &name);

  private:
    Heap heap;
    StringMap strings;
};