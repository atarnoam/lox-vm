#include "src/vm/value.h"
#include "value.h"

#include <fmt/format.h>

// std::string to_string(Value value) {
//     return fmt::format("{:g}", static_cast<double>(value));
// }

Value::Value() : m_type(ValueType::NIL) {}

Value::Value(bool bool_val)
    : m_type(ValueType::BOOL), as({.boolean = bool_val}) {}

Value::Value(double number)
    : m_type(ValueType::NUMBER), as({.number = number}) {}

Value::Value(const Value &other) : m_type(other.m_type) {
    memcpy(&as, &other.as, sizeof(as));
}

ValueType Value::type() const { return m_type; }

bool Value::as_bool() const { return as.boolean; }

double Value::as_number() const { return as.number; }

bool Value::is_bool() const { return m_type == ValueType::BOOL; }

bool Value::is_nil() const { return m_type == ValueType::NIL; }

bool Value::is_number() const { return m_type == ValueType::NUMBER; }

Value::operator bool() const { return !is_nil() and (!is_bool() or as_bool()); }

bool Value::operator==(const Value &other) const {
    if (m_type != other.m_type)
        return false;
    switch (m_type) {
    case ValueType::BOOL:
        return as_bool() == other.as_bool();
    case ValueType::NIL:
        return true;
    case ValueType::NUMBER:
        return as_number() == other.as_number();
    default:
        return false; // Unreachable
    }
}

std::ostream &operator<<(std::ostream &os, const Value &value) {
    switch (value.type()) {
    case ValueType::BOOL: {
        std::ios::fmtflags os_flags(os.flags());
        os.setf(std::ios::boolalpha);
        os << value.as_bool();
        os.flags(os_flags);
        return os;
    }
    case ValueType::NIL:
        return os << "nil";
    case ValueType::NUMBER:
        return os << value.as_number();
    default:
        return os; // Unreachable
    }
}
