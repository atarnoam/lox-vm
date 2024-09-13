#pragma once

#include <string>

struct Obj {
    virtual ~Obj() = default;

    virtual std::string to_string() const;
    virtual bool operator==(const Obj &other) const;
};

struct ObjString : Obj {
    ObjString(std::string string);

    operator const std::string() const;

    std::string to_string() const override;
    bool operator==(const Obj &other) const override;

  private:
    std::string string;
};