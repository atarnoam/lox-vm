#pragma once

#include "src/vm/gc/heap.h"
#include "src/vm/value.h"

#include <string>
#include <string_view>

struct HeapManager {
    HeapManager();

    Value initialize(auto t) { return Value(t); }

    Value initialize(const std::string &string);
    Value initialize(const std::string_view &string);

  private:
    Heap heap;
    StringSet strings;
};