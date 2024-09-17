#include "natives.h"

#include <chrono>
#include <functional>
#include <vector>

Value clock_native(std::span<Value> values) {
    auto now = std::chrono::system_clock::now();
    double seconds_since_epoch =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
            .count();
    return seconds_since_epoch;
}

const std::vector<std::pair<std::string, ObjNative>> NATIVES{
    {"clock", {0, clock_native}}};
