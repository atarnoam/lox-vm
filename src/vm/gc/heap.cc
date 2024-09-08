#include "heap.h"

Heap::Heap() : obj(nullptr) {}

Heap::~Heap() { delete_all(); }

void Heap::delete_all() {
    HeapData *curr = obj;
    while (curr) {
        HeapData *to_delete = curr;
        curr = curr->next;
        delete to_delete;
    }
}
