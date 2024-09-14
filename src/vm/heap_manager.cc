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

heap_ptr<ObjFunction> HeapManager::new_function() {
    return new_function(heap_ptr<ObjString>(nullptr));
}

heap_ptr<ObjFunction> HeapManager::new_function(heap_ptr<ObjString> name) {
    return heap.make<ObjFunction>(0, name, std::move(Chunk{}));
}

heap_ptr<ObjFunction> HeapManager::new_function(const std::string &name) {
    return new_function(initialize(name));
}

heap_ptr<ObjFunction> HeapManager::new_function(const std::string_view &name) {
    return new_function(initialize(name));
}
