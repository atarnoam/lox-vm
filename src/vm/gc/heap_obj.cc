#include "heap_obj.h"

HeapData::HeapData() : marked(false) {}

void HeapData::mark() { marked = true; }

bool HeapData::is_marked() const { return marked; }

void debug_test_size_heap_ptr() {
    if constexpr (DEBUG_SIZEOF_ASSERTS) {
#ifndef __INTELLISENSE__
        // Apparently intellisense doesn't know this...
        // Some long type.
        static_assert(sizeof(heap_ptr<int>) == sizeof(void *));
#endif
    }
}
