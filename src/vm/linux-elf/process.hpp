#pragma once

#include <vector>

#include "../x64/vmem.hpp"
#include "thread.hpp"

class VMState;

constexpr uint8_t PROC_STATE_NEW = 0;
constexpr uint8_t PROC_STATE_RUNNING = 1;
constexpr uint8_t PROC_STATE_WAITING = 2;
constexpr uint8_t PROC_STATE_ZOMBIE = 3;
constexpr uint8_t PROC_STATE_DEAD = 4;

class Process {
   private:
    int32_t _pid;
    int32_t _ppid;
    VirtualMemory _vmem;
    VMState* _vmState = nullptr;
    HeapMemory _heap;
    std::vector<Thread> _threads;
    size_t _currThread = 0;
    size_t _threadCount = 0;
    int _exitStatus = 0;
    uint8_t _state = PROC_STATE_NEW;

   public:
    explicit Process(VMState* state, int32_t pid, int32_t ppid);

    ExecutionContext ctx();

    VirtualMemory& vmem();

    int exitStatus() const;
};