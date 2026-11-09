#pragma once

#include "../../common/config.hpp"
#include "cpu.hpp"
#include "memory.hpp"

struct VMState {
    CPU cpu;
    Memory memory;
    VMConfig config;
};