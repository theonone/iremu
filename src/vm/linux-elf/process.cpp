#include "process.hpp"

#include "../x64/state.hpp"
#include "loader.hpp"

Process::Process(VMState* state, int pid, int ppid) : _pid(pid), _ppid(ppid) {
    _vmem.init(state);
    ExecutableLoader loader;
    loader.init(state);

    _threads.emplace_back(_threadCount++);
}

ExecutionContext Process::ctx() {
    Thread& thread = _threads[_currThread];
    return {.heap = _heap, .stack = thread.stack(), .regs = thread.regs()};
}

VirtualMemory& Process::vmem() { return _vmem; }

int Process::exitStatus() const { return _exitStatus; }

Thread::Thread(int tid) : _tid(tid) {}

StackMemory& Thread::stack() { return _stack; }

Registers& Thread::regs() { return _regs; }
