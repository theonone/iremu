#pragma once
#include "../x64/structs.hpp"

constexpr uint8_t THR_STATE_NEW = 0;
constexpr uint8_t THR_STATE_READY = 1;
constexpr uint8_t THR_STATE_WAITING = 2;
constexpr uint8_t THR_STATE_ZOMBIE = 3;
constexpr uint8_t THR_STATE_DEAD = 4;

class Thread {
   private:
    int32_t _tid;
    Registers _regs;
    StackMemory _stack;
    uint8_t _state = THR_STATE_READY;

   public:
    Thread(int32_t tid, VirtualMemory& vmem);

    uint8_t state() const;
    int32_t tid() const;

    StackMemory& stack();
    Registers& regs();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;
};