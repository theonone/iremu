#pragma once

#include "structs.hpp"

class VMState;

class CPU {
   private:
    VMState* _vmState = nullptr;

   public:
    void run(ExecutionContext ctx, uint64_t n);
    void init(VMState* vmState);
};