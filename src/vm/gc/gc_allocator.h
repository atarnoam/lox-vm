#pragma once

#include "src/debug_flags.h"
#include <functional>
#include <memory>
#include <optional>

namespace gc {
using GCFunc = std::function<void(void)>;
};

template <typename T>
struct GCAllocator : std::allocator<T> {
  private:
    using Base = std::allocator<T>;
    using Pointer = typename std::allocator_traits<Base>::pointer;
    using SizeType = typename std::allocator_traits<Base>::size_type;

    using GCFunc = gc::GCFunc;
    // The shared_ptr is so that we could change the hook using a copy of the
    // allocator.
    std::shared_ptr<std::optional<GCFunc>> gc_hook;

  public:
    GCAllocator()
        : std::allocator<T>(),
          gc_hook(std::make_shared<std::optional<GCFunc>>(std::nullopt)){};

    template <typename U>
    GCAllocator(const GCAllocator<U> &other)
        : Base(other), gc_hook(other.gc_hook) {}

    template <typename U>
    struct rebind {
        using other = GCAllocator<U>;
    };

    void set_gc(GCFunc collect_garbage) { *gc_hook = collect_garbage; }

    Pointer allocate(SizeType n) {
        if constexpr (DEBUG_STRESS_GC) {
            if (*gc_hook) {
                std::invoke(gc_hook->value());
            }
        }
        return Base::allocate(n);
    }
};

template <typename T>
std::vector<T, GCAllocator<T>> new_gc_vector(gc::GCFunc collect_garbage) {
    std::vector<T, GCAllocator<T>> v;
    v.get_allocator().set_gc(collect_garbage);
    return v;
}
