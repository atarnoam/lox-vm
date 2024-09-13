#include "heap_manager.h"

HeapManager::HeapManager() : heap(), strings() {}

Value HeapManager::initialize(const std::string &string) {
    auto string_it = strings.find(string);
    if (string_it == strings.end()) {
        auto object = heap.make<ObjString>(string);
        strings.emplace(string, object);
        return object;
    } else {
        return string_it->second;
    }
    return heap.make<ObjString>(string);
}

Value HeapManager::initialize(const std::string_view &string) {
    return initialize(std::string(string));
}
