#pragma once

#include <unordered_map>

#include "process.hpp"

class ProcessManager {
   private:
    std::unordered_map<int32_t, Process> _processes;
    VMState* _vmState = nullptr;

   public:
    ProcessManager();
    void init(VMState* vmState);

    void createProcess(int32_t pid, const std::string& elf, int32_t ppid);

    // @returns exit code of the target
    int run();
};