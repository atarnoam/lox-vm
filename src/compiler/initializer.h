#pragma once

#include "src/vm/gc/heap.h"
#include "src/vm/value.h"

struct Initializer {
    Initializer(Heap &heap, StringSet &strings);

    Value initialize(auto t) { return Value(t); }

    Value initialize(const std::string &string);

  private:
    Heap &heap;
    StringSet &strings;
};