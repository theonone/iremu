#pragma once

#include "structs.hpp"

class CPU {
   private:
    Registers _regs;

   public:
    void step();
    void run();
    const Registers& regs() const;
};