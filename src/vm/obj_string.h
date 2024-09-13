#pragma once

#include <string>
#include <type_traits>
#include <utility>

namespace obj_string {
constinit inline auto hash_func = std::hash<std::string>{};
using hash_t = std::invoke_result_t<decltype(hash_func), std::string>;
}; // namespace obj_string

struct ObjString {
    ObjString(std::string s);

    operator std::string() const;

    bool operator==(const ObjString &other) const;

    obj_string::hash_t hash() const;

  private:
    std::string string;
    obj_string::hash_t m_hash;
};

struct ObjStringHash {
    obj_string::hash_t operator()(const ObjString &string);
};
