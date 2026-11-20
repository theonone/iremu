#pragma once

#include <unordered_map>

#include "process.hpp"

class VMState;

class ProcessManager {
   private:
    std::unordered_map<int32_t, Process> _processes;
    VMState* _state = nullptr;

   public:
    ProcessManager();
    void init(VMState* state);
    void run();
};