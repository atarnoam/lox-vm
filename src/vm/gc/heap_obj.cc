#include "heap_obj.h"

HeapData::HeapData() : HeapData(nullptr) {}

HeapData::HeapData(HeapData *next) : next(next), marked(false) {}

void debug_test_size_heap_ptr() {
    if constexpr (DEBUG_SIZEOF_ASSERTS) {
#ifndef __INTELLISENSE__
        // Apparently intellisense doesn't know this...
        // Some long type.
        static_assert(sizeof(heap_ptr<int>) == sizeof(void *));
#endif
    }
}
