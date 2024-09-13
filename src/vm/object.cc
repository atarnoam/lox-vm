#include "object.h"

ObjString::ObjString(std::string string) : string(std::move(string)) {}

ObjString::operator const std::string() const { return string; }

std::string ObjString::to_string() const { return string; }
