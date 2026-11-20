#pragma once
#include "vmem.hpp"

constexpr uint64_t STACK_TOP = 0x00007FFFFFFFF000;

class StackMemory {
   private:
    uint64_t _maxSize;
    VirtualMemory& _vmem;

   public:
    StackMemory(VirtualMemory& vmem);

    void push(uint64_t val);
    uint64_t pop();

    void checkRbp(uint64_t rbp);

    uint8_t read8(uint64_t vaddr);
    uint16_t read16(uint64_t vaddr);
    uint32_t read32(uint64_t vaddr);
    uint64_t read64(uint64_t vaddr);

    void write8(uint64_t vaddr, uint8_t value);
    void write16(uint64_t vaddr, uint16_t value);
    void write32(uint64_t vaddr, uint32_t value);
    void write64(uint64_t vaddr, uint64_t value);
};