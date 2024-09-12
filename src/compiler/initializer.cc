#include "initializer.h"

Initializer::Initializer(Heap &heap, StringSet &strings)
    : heap(heap), strings(strings) {}

Value Initializer::initialize(const std::string &string) {
    /** TODO: interning... */
    return heap.make<ObjString>(string);
}
