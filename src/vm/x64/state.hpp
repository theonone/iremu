#pragma once

#include "../../common/config.hpp"
#include "cpu.hpp"
#include "pmem.hpp"

struct VMState {
    CPU cpu;
    PhysicalMemory ram;
    VMConfig config;
};