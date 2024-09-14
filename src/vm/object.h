#pragma once

#include <string>
#include <type_traits>
#include <utility>

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
