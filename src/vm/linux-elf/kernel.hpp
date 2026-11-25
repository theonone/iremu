#pragma once

#include "../x64/structs.hpp"

class Kernel {
   private:
   public:
    void syscall(ExecutionContext ctx);
    uint64_t syscallPrice(ExecutionContext);
};