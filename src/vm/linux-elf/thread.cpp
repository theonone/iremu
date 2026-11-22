#include "thread.hpp"

#include "../cfgProv.hpp"

Thread::Thread(int32_t tid, VirtualMemory& vmem)
    : _tid(tid),
      _stack(vmem, GLB_STACK_TOP - (ConfigProvider::get().max_stack * tid),
             ConfigProvider::get().max_stack) {}

uint8_t Thread::state() const { return _state; }

Registers& Thread::regs() { return _regs; }

int32_t Thread::tid() const { return _tid; }
