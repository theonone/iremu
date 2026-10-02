#pragma once
#include <cstdint>
#include <optional>
#include <string>

struct VMConfig {
    int64_t max_cycles;
    int64_t max_heap;
    int64_t max_stack;
    std::optional<std::string> out;
    std::optional<std::string> in;
    std::string args;
    bool verbose;
};
