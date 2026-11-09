#pragma once

#include "../../common/config.hpp"
#include "../x64/state.hpp"

class VirtualMachine {
   private:
    VMState _state;

   public:
    VirtualMachine(const VMConfig& config);
};