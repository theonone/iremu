#pragma once
#include "vmem.hpp"

constexpr uint64_t STACK_TOP = 0x00007FFFFFFFF000;
constexpr uint64_t REDZONE = 128;

class StackMemory {
   private:
    uint64_t _maxSize;
    VirtualMemory& _vmem;
    uint64_t _rsp = STACK_TOP;
    uint64_t _lowestAddr = STACK_TOP;
    uint64_t _pageSize;

    void _checkPages();
    inline void _validateVaddr(uint64_t vaddr, uint64_t size);

   public:
    StackMemory(VirtualMemory& vmem);
    ~StackMemory();

    void push(uint64_t val);
    uint64_t pop();

    void updateRsp(uint64_t rsp);  // called after each cpu cycle

    uint8_t read8(uint64_t vaddr);
    uint16_t read16(uint64_t vaddr);
    uint32_t read32(uint64_t vaddr);
    uint64_t read64(uint64_t vaddr);

    void write8(uint64_t vaddr, uint8_t value);
    void write16(uint64_t vaddr, uint16_t value);
    void write32(uint64_t vaddr, uint32_t value);
    void write64(uint64_t vaddr, uint64_t value);
};