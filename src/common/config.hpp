#pragma once
#include <cstdint>
#include <optional>
#include <string>

struct VMConfig {
    int64_t max_cycles;  // max CPU cycles to execute before forced termination, -1 for unlimited
    ssize_t max_heap;    // max heap memory an executable can allocate, -1 for unlimited
    ssize_t max_stack;   // max stack memory an executable can allocate, -1 for unlimited
    size_t ram_size;     // total amount of memory the executable gets, including for storage of the
                         // code, sections, stack, heap, etc. must be an unsigned integer
    std::optional<std::string> out;  // filepath to write stdout into or nullopt for normal stdout
    std::optional<std::string> in;   // same as out, but for stdin
    std::string args;                // args to pass to the executable
    bool verbose;                    // whether to print VM event messages or not
};
