#pragma once

#include "structs.hpp"

class VMState;

class CPU {
   private:
    Registers _regs;
    VMState* _state = nullptr;

   public:
    void step();
    void run();
    const Registers& regs() const;
    void init(VMState* state);
};