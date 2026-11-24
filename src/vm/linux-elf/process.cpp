#include "process.hpp"

// #include "../cfgProv.hpp"
#include <iostream>

#include "../x64/vmState.hpp"
#include "loader.hpp"
#include "thread.hpp"

Process::Process(VMState* vmState, int32_t pid, int32_t ppid, const std::string& elf)
    : _pid(pid), _ppid(ppid), _heap(&_vmem) {
    _vmem.init(vmState);

    _threads.push_back(new Thread(_nextTid++, _vmem));

    auto& thr = _threads[0];
    {
        ExecutableLoader loader(
            elf, &_vmem,
            {.heap = _heap, .stack = thr->stack(), .regs = thr->regs(), .pid = 0, .tid = 0});
    }
    std::cout << "ELF loaded successfully. heap base=" << _heap.base()
              << ", rip=" << thr->regs().rip << ", rsp=" << thr->regs().rsp
              << ", rbp=" << thr->regs().rbp << std::endl;

    std::cout << "vmem page map:" << std::endl;
    for (const auto& entry : _vmem.pageMap()) {
        std::string perm;
        if (entry.second.perm & MEM_PERM_R)
            perm += 'R';
        if (entry.second.perm & MEM_PERM_W)
            perm += 'W';
        if (entry.second.perm & MEM_PERM_X)
            perm += 'X';

        std::cout << entry.first << "-" << entry.first + entry.second.pages * 4096 << ", " << perm
                  << std::endl;
    }
    _state = PROC_STATE_READY;
}

Process::~Process() {
    for (const auto ptr : _threads) {
        delete ptr;
    }
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

        Thread* thr = _threads[thisThread];
        if (thr->state() == THR_STATE_READY) {
            return ExecutionContext{
                .heap = _heap,
                .stack = thr->stack(),
                .regs = thr->regs(),
                .pid = _pid,
                .tid = thr->tid(),
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
