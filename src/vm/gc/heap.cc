#include "heap.h"

Heap::Heap() : objects{} {}

Heap::~Heap() { delete_all(); }

void Heap::delete_all() {
    while (!objects.empty()) {
        delete objects.front();
        objects.erase_after(objects.before_begin());
    }
}
