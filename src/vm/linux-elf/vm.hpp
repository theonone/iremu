#pragma once

#include "../../common/config.hpp"
#include "../x64/vmState.hpp"
#include "procManager.hpp"

class VirtualMachine {
   private:
    VMState _vmState;
    ProcessManager _procManager;

   public:
    VirtualMachine(const VMConfig& config);

    int run();
};