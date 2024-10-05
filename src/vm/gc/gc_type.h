#pragma once

#include <array>
#include <ranges>
#include <utility>

template <typename T>
concept GCable = requires(T t) {
    { t.gc_children() } -> std::ranges::range;
};

template <typename T>
struct EmptyGC {
    EmptyGC(T t) : t(std::move(t)) {}
    operator T &() { return t; }

    std::array<T, 0> gc_children() { return {}; }

    T t;
};
