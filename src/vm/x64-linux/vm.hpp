#pragma once

#include "../../common/config.hpp"
#include "state.hpp"

class VirtualMachine {
   private:
    VMState _state;

   public:
    VirtualMachine(const VMConfig& config);
};