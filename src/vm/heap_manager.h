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

    heap_ptr<ObjFunction> new_function();
    heap_ptr<ObjFunction> new_function(heap_ptr<ObjString> name);
    heap_ptr<ObjFunction> new_function(const std::string &name);
    heap_ptr<ObjFunction> new_function(const std::string_view &name);

  private:
    Heap heap;
    StringMap strings;
};