#pragma once
#include <cstdint>
#include <optional>
#include <string>

struct VMConfig {
    int64_t max_cycles;
    int64_t max_heap;
    int64_t max_stack;
    std::optional<std::string> stdout;
    std::optional<std::string> stdin;
    std::string args;
    bool verbose;
};
