#pragma once

#include "src/debug_flags.h"
#include "src/util/type_name.h"
#include <cstddef>
#include <fmt/format.h>
#include <iostream>
#include <utility>

struct HeapData {
    HeapData();

    virtual ~HeapData() = default;

    void mark();
    bool is_marked() const;

    friend class Heap;

  private:
    bool marked;
};

template <typename T>
struct HeapObj : HeapData {
    template <typename... Args>
    HeapObj(Args... args) : obj(std::forward<Args>(args)...) {}

    ~HeapObj() {
        if constexpr (DEBUG_LOG_GC) {
            std::cout << fmt::format("{:p} free {}\n",
                                     static_cast<void *>(this), type_name<T>());
        }
    }

    T obj;
};

template <typename T>
struct heap_ptr {
    heap_ptr(HeapObj<T> *ptr) : ptr(ptr) {}
    heap_ptr(nullptr_t) : ptr(nullptr) {}

    const T &operator*() const { return ptr->obj; };
    T &operator*() { return ptr->obj; }
    const T *operator->() const { return &(ptr->obj); }
    T *operator->() { return &(ptr->obj); }

    bool operator==(heap_ptr<T> other) { return ptr == other.ptr; }
    bool operator==(nullptr_t other) { return ptr == other; }

    // Note we don't delete ptr here.
    ~heap_ptr() = default;

  protected:
    operator HeapObj<T> *() { return ptr; }

  private:
    HeapObj<T> *ptr;
};

// if debug flag is on, asserts that sizeof(heap_ptr) == sizeof(void*)
void debug_test_size_heap_ptr();