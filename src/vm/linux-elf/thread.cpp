#include "thread.hpp"

Thread::Thread(int32_t tid) : _tid(tid) {}

Registers& Thread::regs() { return _regs; }
