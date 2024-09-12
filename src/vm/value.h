#pragma once

#include <iostream>
#include <unordered_set>
#include <vector>

#include "src/vm/gc/heap_obj.h"
#include "src/vm/obj_string.h"

enum struct ValueType { BOOL, NIL, NUMBER, STRING };

struct Value {
    /// @brief Initialize to nil.
    Value();
    Value(bool boolean);
    Value(double number);
    Value(heap_ptr<ObjString> string);

    Value(const Value &other);

    ValueType type() const;

    bool as_bool() const;
    double as_number() const;
    heap_ptr<ObjString> as_string() const;

    bool is_bool() const;
    bool is_nil() const;
    bool is_number() const;
    bool is_string() const;

    /// @brief The only false types are nil and "false".
    operator bool() const;

    bool operator==(const Value &other) const;

  private:
    ValueType m_type;
    union ValueU {
        bool boolean;
        double number;
        heap_ptr<ObjString> string;

        ValueU();
        ValueU(bool boolean);
        ValueU(double number);
        ValueU(heap_ptr<ObjString> string);
    } as;

#ifndef __INTELLISENSE__
    // Apparently intellisense doesn't know this...
    static_assert(sizeof(ValueU) == sizeof(double));
#endif
};

using ValueArray = std::vector<Value>;

std::ostream &operator<<(std::ostream &os, const Value &value);

constinit auto STRING_HASH = [](heap_ptr<ObjString> string) {
    return string->hash();
};

constinit auto STRING_EQ = [](heap_ptr<ObjString> string1,
                              heap_ptr<ObjString> string2) {
    return *string1 == *string2;
};

/** Used for string interning. */
using StringSet = std::unordered_set<heap_ptr<ObjString>, decltype(STRING_HASH),
                                     decltype(STRING_EQ)>;
