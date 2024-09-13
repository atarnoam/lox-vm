#include "heap_manager.h"

HeapManager::HeapManager() : heap(), strings() {}

Value HeapManager::initialize(const std::string &string) {
    auto string_it = strings.find(string);
    if (string_it == strings.end()) {
        auto obj_string = heap.make<ObjString>(string);
        strings.emplace(string, obj_string);
        return obj_string;
    } else {
        return string_it->second;
    }
    return heap.make<ObjString>(string);
}

Value HeapManager::initialize(const std::string_view &string) {
    return initialize(std::string(string));
}
