#include "thread.hpp"

#include "../cfgProv.hpp"

Thread::Thread(int32_t tid, VirtualMemory& vmem)
    : _tid(tid),
      _stack(vmem, GLB_STACK_TOP - (ConfigProvider::get().max_stack * tid),
             ConfigProvider::get().max_stack) {
    _regs.rbp = _stack.topVaddr();
    _regs.rsp = _regs.rbp;
}

uint8_t Thread::state() const { return _state; }

StackMemory& Thread::stack() { return _stack; }

Registers& Thread::regs() { return _regs; }

int32_t Thread::tid() const { return _tid; }
