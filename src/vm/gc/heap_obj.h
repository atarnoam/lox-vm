#pragma once

#include <cstddef>
#include <utility>

struct HeapData {
    HeapData();
    HeapData(HeapData *next);

    virtual ~HeapData() = default;

    friend class Heap;

  private:
    HeapData *next;
    bool marked;
};

template <typename T>
struct HeapObj : HeapData {
    template <typename... Args>
    HeapObj(HeapData *next, Args... args)
        : HeapData(next), obj(std::forward<Args>(args)...) {}

    T obj;
};

template <typename T>
struct heap_ptr {
    heap_ptr(HeapObj<T> *ptr) : ptr(ptr) {}
    heap_ptr(nullptr_t) : ptr(nullptr) {}

    const T &operator*() const;
    // T &operator*() { return ptr->obj; }
    const T *operator->() const { return &(ptr->obj); }
    T *operator->() { return &(ptr->obj); }

    // Note we don't delete ptr here.
    ~heap_ptr() = default;

  private:
    HeapObj<T> *ptr;
};

template <typename T>
inline const T &heap_ptr<T>::operator*() const {
    return ptr->obj;
}
