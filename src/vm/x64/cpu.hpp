#pragma once

#include "structs.hpp"

class VMState;

constexpr uint8_t CPU_INT_SYSCALL = 0;
constexpr uint8_t CPU_INT_CYCLES = 1;

struct CPUInterrupt {
    uint8_t reason;
    uint64_t remaining;
};

class CPU {
   private:
    VMState* _vmState = nullptr;

   public:
    CPUInterrupt run(ExecutionContext ctx, uint64_t n);
    void init(VMState* vmState);
};