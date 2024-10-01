#pragma once

#include <functional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

struct Value;

namespace object {
constinit inline auto str_hash_func = std::hash<std::string>{};
using hash_t = std::invoke_result_t<decltype(str_hash_func), std::string>;
}; // namespace object

struct ObjString {
    ObjString(std::string s);

    operator std::string() const;
    const std::string &str() const;

    bool operator==(const ObjString &other) const;

    object::hash_t hash() const;

  private:
    std::string string;
    object::hash_t m_hash;
};

struct ObjStringHash {
    object::hash_t operator()(const ObjString &string);
};

using NativeFn = std::function<Value(std::span<Value>)>;

struct ObjNative {
    int arity;
    NativeFn fun;
};

struct UpValue {
    heap_ptr<Value> location;
};