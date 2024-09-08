#include <gtest/gtest.h>

#include "src/vm/gc/heap.h"
#include "src/vm/gc/heap_obj.h"
#include <string>

struct A {
    int x;
};

TEST(HeapTests, TestAll) {
    Heap heap{};
    auto p = heap.make<std::string>("abc");
    auto p2 = p;

    auto q = heap.make<A>(10);

    std::cout << *p2 + *p << q->x << std::endl;
}