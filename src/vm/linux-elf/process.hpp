#pragma once

#include <vector>

#include "../x64/structs.hpp"
#include "../x64/vmem.hpp"

class VMState;

constexpr uint8_t PROC_STATE_RUNNING = 0;
constexpr uint8_t PROC_STATE_WAITING = 1;
constexpr uint8_t PROC_STATE_ZOMBIE = 2;
constexpr uint8_t PROC_STATE_DEAD = 3;

class Thread {
   private:
    int _tid;
    Registers _regs;
    StackMemory _stack;

   public:
    Thread(int tid);

    StackMemory& stack();
    Registers& regs();
};

class Process {
   private:
    int _pid;
    int _ppid;
    VirtualMemory _vmem;
    VMState* _state = nullptr;
    HeapMemory _heap;
    std::vector<Thread> _threads;
    size_t _currThread = 0;
    size_t _threadCount = 0;
    int _exitStatus = 0;

   public:
    explicit Process(VMState* state, int pid, int ppid);

    ExecutionContext ctx();

    VirtualMemory& vmem();

    int exitStatus() const;
};