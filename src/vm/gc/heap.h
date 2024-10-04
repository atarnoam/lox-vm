#pragma once

#include "src/vm/gc/heap_obj.h"
#include <forward_list>
#include <utility>

struct Heap {
    Heap();

    template <typename T, typename... Args>
    heap_ptr<T> make(Args &&...args) {
        HeapObj<T> *ptr = new HeapObj<T>(std::forward<Args>(args)...);
        if (!ptr) {
            return nullptr;
        }
        objects.push_front(ptr);
        return ptr;
    }

    Heap(const Heap &) = delete;
    Heap &operator=(const Heap &) = delete;
    Heap(Heap &&) = delete;
    Heap &operator=(Heap &&) = delete;

    ~Heap();

  private:
    void delete_all();

    std::forward_list<HeapData *> objects;
};