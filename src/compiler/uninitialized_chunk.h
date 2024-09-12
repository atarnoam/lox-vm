#pragma once

#include "src/vm/code.h"
#include "src/vm/gc/heap.h"
#include "src/vm/value.h"
#include <tuple>

struct IValueInitializer {
    virtual Value initialize(const Heap &heap) const = 0;
};

template <typename T, typename... Args>
concept HeapValueInitializerConcept = requires(const Heap &heap,
                                               const Args &&...args) {
    Value(heap.make<T, Args...>(args...));
};

template <typename T>
concept SimpleValueInitializerConcept = requires(const T &&t) {
    Value(t);
};

template <typename T, typename... Args>
requires HeapValueInitializerConcept<T, Args...>
struct ValueInitializer : IValueInitializer {
    ValueInitializer(Args... args) : args(std::move(args)...) {}

    Value initialize(const Heap &heap) const override {
        heap_ptr<T> ptr = std::apply(heap.make<T, Args...>, args);
        return Value(ptr);
    }

  private:
    std::tuple<Args...> args;
};

// template <typename T>
// requires SimpleValueInitializerConcept<T>
// struct ValueInitializer : IValueInitializer {
//     ValueInitializer(T t) : t(std::move(t)) {}

//     Value initialize(const Heap &heap) const override {
//         return Value(heap.make<T>(args...));
//     }

//   private:
//     T t;
// };

ValueInitializer<ObjString, std::string> v(std::string("abc"));

/** Chunk which doesn't contain `Values`, but things that can be initialized to
 * values.
 */
struct UninitializedChunk : CodeChunk {};