#include "heap_manager.h"

HeapManager::HeapManager() : heap(), strings() {}

Value HeapManager::initialize(const std::string &string) {
    /** TODO: interning... */
    return heap.make<ObjString>(string);
}

Value HeapManager::initialize(const std::string_view &string) {
    return initialize(std::string(string));
}
