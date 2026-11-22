#pragma once

#include <optional>
#include <vector>

#include "../x64/vmem.hpp"
#include "thread.hpp"

class VMState;

constexpr uint8_t PROC_STATE_NEW = 0;
constexpr uint8_t PROC_STATE_READY = 1;
constexpr uint8_t PROC_STATE_WAITING = 2;
constexpr uint8_t PROC_STATE_ZOMBIE = 3;
constexpr uint8_t PROC_STATE_DEAD = 4;

class Process {
   private:
    int32_t _pid;
    int32_t _ppid;
    VirtualMemory _vmem;
    VMState* _vmState = nullptr;
    uint64_t _stackMargin;
    uint64_t _stackLowest;
    HeapMemory _heap;
    std::vector<Thread> _threads;
    int32_t _nextTid = 0;
    int _exitCode = 0;
    uint8_t _state = PROC_STATE_NEW;
    size_t _nextThread = 0;  // which thread to give CPU time to

   public:
    explicit Process(VMState* vmState, int32_t pid, int32_t ppid, const std::string& elf);

    std::optional<ExecutionContext> ctx();

    VirtualMemory& vmem();

    uint8_t state() const;
    int32_t pid() const;
    int32_t ppid() const;

    int exitCode() const;
};