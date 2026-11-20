#pragma once
#include "../x64/structs.hpp"

class Thread {
   private:
    int32_t _tid;
    Registers _regs;
    StackMemory _stack;

   public:
    Thread(int32_t tid);

    StackMemory& stack();
    Registers& regs();
};