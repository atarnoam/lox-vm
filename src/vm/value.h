#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "src/vm/gc/heap_obj.h"
#include "src/vm/obj_function.fwd.h"
#include "src/vm/object.h"

enum struct ValueType { BOOL, NIL, NUMBER, STRING, FUNCTION, NATIVE, CLOSURE };

struct Value {
    /// @brief Initialize to nil.
    Value();
    Value(bool boolean);
    Value(double number);
    Value(heap_ptr<ObjString> string);
    Value(heap_ptr<ObjFunction> function);
    Value(heap_ptr<ObjNative> native);
    Value(heap_ptr<ObjClosure> closure);

    Value(const Value &other);

    ValueType type() const;

    bool as_bool() const;
    double as_number() const;
    heap_ptr<ObjString> as_string() const;
    heap_ptr<ObjFunction> as_function() const;
    heap_ptr<ObjNative> as_native() const;
    heap_ptr<ObjClosure> as_closure() const;

    /// @brief The only false types are nil and "false".
    operator bool() const;

    operator double() const;
    operator heap_ptr<ObjString>() const;
    operator heap_ptr<ObjFunction>() const;
    operator heap_ptr<ObjNative>() const;
    operator heap_ptr<ObjClosure>() const;

    bool is_bool() const;
    bool is_nil() const;
    bool is_number() const;
    bool is_string() const;
    bool is_function() const;
    bool is_native() const;
    bool is_closure() const;

    bool operator==(const Value &other) const;

  private:
    ValueType m_type;
    union ValueU {
        bool boolean;
        double number;
        heap_ptr<ObjString> string;
        heap_ptr<ObjFunction> function;
        heap_ptr<ObjNative> native;
        heap_ptr<ObjClosure> closure;

        ValueU();
        ValueU(bool boolean);
        ValueU(double number);
        ValueU(heap_ptr<ObjString> string);
        ValueU(heap_ptr<ObjFunction> function);
        ValueU(heap_ptr<ObjNative> native);
        ValueU(heap_ptr<ObjClosure> closure);
    } as;

#ifndef __INTELLISENSE__
    // Apparently intellisense doesn't know this...
    static_assert(sizeof(ValueU) == sizeof(double));
#endif
};

using ValueArray = std::vector<Value>;

std::ostream &operator<<(std::ostream &os, const Value &value);

constinit inline auto STRING_HASH = [](heap_ptr<ObjString> string) {
    return string->hash();
};

constinit inline auto STRING_EQ = [](heap_ptr<ObjString> string1,
                                     heap_ptr<ObjString> string2) {
    return *string1 == *string2;
};

/** Used for string interning. */
using StringMap = std::unordered_map<std::string, heap_ptr<ObjString>,
                                     decltype(object::str_hash_func)>;

using VariableMap =
    std::unordered_map<heap_ptr<ObjString>, Value, decltype(STRING_HASH),
                       decltype(STRING_EQ)>;
