#include "value.h"

#include "src/vm/obj_function.h"
#include "src/vm/obj_upvalue.h"
#include <fmt/format.h>

Value::Value() : m_type(ValueType::NIL) {}

Value::Value(bool bool_val) : m_type(ValueType::BOOL), as(bool_val) {}

Value::Value(double number) : m_type(ValueType::NUMBER), as(number) {}

Value::Value(heap_ptr<ObjString> string)
    : m_type(ValueType::STRING), as(string) {}

Value::Value(heap_ptr<ObjFunction> function)
    : m_type(ValueType::FUNCTION), as(function) {}

Value::Value(heap_ptr<ObjNative> native)
    : m_type(ValueType::NATIVE), as(native) {}

Value::Value(heap_ptr<ObjClosure> closure)
    : m_type(ValueType::CLOSURE), as(closure) {}

Value::Value(heap_ptr<ObjUpvalue> upvalue)
    : m_type(ValueType::UPVALUE), as(upvalue) {}

Value::Value(const Value &other) : m_type(other.m_type) {
    memcpy(&as, &other.as, sizeof(as));
}

ValueType Value::type() const { return m_type; }

bool Value::as_bool() const { return as.boolean; }

double Value::as_number() const { return as.number; }

heap_ptr<ObjString> Value::as_string() const { return as.string; }

heap_ptr<ObjFunction> Value::as_function() const { return as.function; }

heap_ptr<ObjNative> Value::as_native() const { return as.native; }

heap_ptr<ObjClosure> Value::as_closure() const { return as.closure; }

heap_ptr<ObjUpvalue> Value::as_upvalue() const { return as.upvalue; }

Value::operator double() const { return as_number(); }

Value::operator heap_ptr<ObjString>() const { return as_string(); }

Value::operator heap_ptr<ObjFunction>() const { return as_function(); }

Value::operator heap_ptr<ObjNative>() const { return as_native(); }

Value::operator heap_ptr<ObjClosure>() const { return as_closure(); }

Value::operator heap_ptr<ObjUpvalue>() const { return as_upvalue(); }

bool Value::is_bool() const { return m_type == ValueType::BOOL; }

bool Value::is_nil() const { return m_type == ValueType::NIL; }

bool Value::is_number() const { return m_type == ValueType::NUMBER; }

bool Value::is_string() const { return m_type == ValueType::STRING; }

bool Value::is_function() const { return m_type == ValueType::FUNCTION; }

bool Value::is_native() const { return m_type == ValueType::NATIVE; }

bool Value::is_closure() const { return m_type == ValueType::CLOSURE; }

bool Value::is_upvalue() const { return m_type == ValueType::UPVALUE; }

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
    case ValueType::STRING:
        // We are assuming string interning here!
        return as_string() == other.as_string();
    case ValueType::FUNCTION:
        return as_function() == other.as_function();
    case ValueType::NATIVE:
        return as_native() == other.as_native();
    case ValueType::CLOSURE:
        return as_closure() == other.as_closure();
    case ValueType::UPVALUE:
        return as_upvalue() == other.as_upvalue();
    }

    throw std::runtime_error("Unexpected Value type");
}

void Value::mark() {
#define MARK_AND_LOG(ptr)                                                      \
    {                                                                          \
        ptr.mark();                                                            \
        if constexpr (DEBUG_LOG_GC) {                                          \
            std::cout << static_cast<void *>(ptr.get()) << " mark " << *this   \
                      << "\n";                                                 \
        }                                                                      \
    }
    switch (m_type) {
    case ValueType::BOOL:
    case ValueType::NIL:
    case ValueType::NUMBER:
        break;
    case ValueType::STRING: {
        auto ptr = as_string();
        MARK_AND_LOG(ptr);
    } break;
    case ValueType::FUNCTION: {
        auto ptr = as_function();
        MARK_AND_LOG(ptr);
    } break;
    case ValueType::NATIVE: {
        auto ptr = as_native();
        MARK_AND_LOG(ptr);
    } break;
    case ValueType::CLOSURE: {
        auto ptr = as_closure();
        MARK_AND_LOG(ptr);
    } break;
    case ValueType::UPVALUE: {
        auto ptr = as_upvalue();
        MARK_AND_LOG(ptr);
    } break;
    default:
        break;
    }
#undef MARK_AND_LOG
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
    case ValueType::STRING:
        return os << value.as_string()->str();
    case ValueType::FUNCTION:
        if (value.as_function()->name == nullptr) {
            return os << "<script>";
        }
        return os << fmt::format("<fn {}>", value.as_function()->name->str());
    case ValueType::NATIVE:
        // TODO: add names to natives.
        return os << "<native fn>";
    case ValueType::CLOSURE:
        return os << Value(value.as_closure()->function);
    case ValueType::UPVALUE:
        return os << "upvalue";
    }

    throw std::runtime_error("Unexpected Value type");
}

Value::ValueU::ValueU() : boolean(false) {}

Value::ValueU::ValueU(bool boolean) : boolean(boolean) {}

Value::ValueU::ValueU(double number) : number(number) {}

Value::ValueU::ValueU(heap_ptr<ObjString> string) : string(string) {}

Value::ValueU::ValueU(heap_ptr<ObjFunction> function) : function(function) {}

Value::ValueU::ValueU(heap_ptr<ObjNative> native) : native(native) {}

Value::ValueU::ValueU(heap_ptr<ObjClosure> closure) : closure(closure) {}

Value::ValueU::ValueU(heap_ptr<ObjUpvalue> upvalue) : upvalue(upvalue) {}
