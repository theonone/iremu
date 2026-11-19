#pragma once

#include "../x64/structs.hpp"
#include "../x64/vmem.hpp"

struct ExecutionContext {
    VirtualMemory& vmem;
    Registers& regs;
};