#include "thread.hpp"

Thread::Thread(int32_t tid) : _tid(tid) {}

uint8_t Thread::state() const { return _state; }

Registers& Thread::regs() { return _regs; }
