#include "obj_string.h"

using namespace obj_string;

ObjString::ObjString(std::string s)
    : string(std::move(s)), m_hash(hash_func(string)) {}

ObjString::operator std::string() const { return string; }

bool ObjString::operator==(const ObjString &other) const {
    return string == other.string;
}

hash_t ObjString::hash() const { return m_hash; }

hash_t ObjStringHash::operator()(const ObjString &string) {
    return string.hash();
}
