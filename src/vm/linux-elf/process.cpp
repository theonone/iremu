#include "process.hpp"

#include "../x64/state.hpp"
#include "loader.hpp"
#include "thread.hpp"

Process::Process(int32_t pid, int32_t ppid) : _pid(pid), _ppid(ppid) {
    ExecutableLoader loader;

    _threads.emplace_back(_threadCount++);
}

ExecutionContext Process::ctx() {
    Thread& thread = _threads[_currThread];
    return {.heap = _heap, .stack = thread.stack(), .regs = thread.regs()};
}

VirtualMemory& Process::vmem() { return _vmem; }

int Process::exitStatus() const { return _exitStatus; }

StackMemory& Thread::stack() { return _stack; }
