#pragma once

#include <iostream>
#include <vector>

enum struct ValueType { BOOL, NIL, NUMBER };

struct Value {
    /// @brief Initialize to nil.
    Value();
    Value(bool boolean);
    Value(double number);

    Value(const Value &other);

    ValueType type() const;

    bool as_bool() const;
    double as_number() const;

    bool is_bool() const;
    bool is_nil() const;
    bool is_number() const;

  private:
    ValueType m_type;
    union ValueU {
        bool boolean;
        double number;
    } as;
};

using ValueArray = std::vector<Value>;

std::ostream &operator<<(std::ostream &os, const Value &value);