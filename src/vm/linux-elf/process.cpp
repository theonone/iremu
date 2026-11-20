#include "process.hpp"

#include "../x64/vmState.hpp"
#include "loader.hpp"
#include "thread.hpp"

Process::Process(VMState* vmState, int32_t pid, int32_t ppid, const std::string& elf)
    : _pid(pid), _ppid(ppid) {
    _vmem.init(vmState);
    _threads.emplace_back(0, _vmem);
    _threads.emplace_back(1, _vmem);

    ExecutableLoader loader;
    loader.load(elf, &_vmem, &(_threads[0].regs()));

    _state = PROC_STATE_READY;
}

std::optional<ExecutionContext> Process::ctx() {
    size_t orig = _nextThread;

    // should basically never happen, but fine
    if (_threads.empty())
        return std::nullopt;

    while (true) {
        size_t thisThread = _nextThread;

        ++_nextThread;
        if (_nextThread == _threads.size()) {
            _nextThread = 0;
        }

        Thread& thr = _threads[thisThread];
        if (thr.state() == THR_STATE_READY) {
            return ExecutionContext{
                .heap = _heap,
                .stack = thr.stack(),
                .regs = thr.regs(),
                .pid = _pid,
                .tid = thr.tid(),
            };
        }

        if (_nextThread == orig) {  // looped back to the initial => no ready threads
            return std::nullopt;
        }
    }
}

VirtualMemory& Process::vmem() { return _vmem; }

uint8_t Process::state() const { return _state; }

int32_t Process::pid() const { return _pid; }
int32_t Process::ppid() const { return _ppid; }

int Process::exitCode() const { return _exitCode; }

StackMemory& Thread::stack() { return _stack; }
