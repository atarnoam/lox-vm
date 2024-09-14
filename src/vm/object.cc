#include "object.h"

using namespace object;

ObjString::ObjString(std::string s)
    : string(std::move(s)), m_hash(str_hash_func(string)) {}

ObjString::operator std::string() const { return string; }

const std::string &ObjString::str() const { return string; }

bool ObjString::operator==(const ObjString &other) const {
    return string == other.string;
}

hash_t ObjString::hash() const { return m_hash; }

hash_t ObjStringHash::operator()(const ObjString &string) {
    return string.hash();
}
