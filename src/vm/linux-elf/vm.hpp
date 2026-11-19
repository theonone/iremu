#pragma once

#include <unordered_map>

#include "../../common/config.hpp"
#include "../x64/state.hpp"
#include "process.hpp"

class VirtualMachine {
   private:
    VMState _state;
    std::unordered_map<int, Process> _processes;

   public:
    VirtualMachine(const VMConfig& config);

    int run();
};