#pragma once

#include "structs.hpp"

class VMState;

constexpr uint8_t PROC_STATE_RUNNING = 0;
constexpr uint8_t PROC_STATE_WAITING = 1;
constexpr uint8_t PROC_STATE_ZOMBIE = 2;
constexpr uint8_t PROC_STATE_DEAD = 3;

class Process {
   private:
    int _id;
    Process* _parent;
    Registers _regs;
    VirtualMemory _vmem;
    VMState* _state = nullptr;

   public:
    Process(VMState* state, int id, Process* parent);

    ExecutionContext ctx();

    VirtualMemory& vmem();
};