#include <gtest/gtest.h>

#include "src/vm/value.h"
#include <string>

TEST(ValueTests, TestValueString) {
    Value val1("abc");
    Value val2 = val1;
    Value val3(std::move(val1));
    std::cout << val2 << std::endl;
    std::cout << val3 << std::endl;
    std::cout << val2 << std::endl;
}