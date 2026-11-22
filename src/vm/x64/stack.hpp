#pragma once
#include "vmem.hpp"

constexpr uint64_t GLB_STACK_TOP = 0x00007FFFFFFFF000;  //~128TB
constexpr uint64_t REDZONE = 128;

class StackMemory {
   private:
    VirtualMemory& _vmem;
    uint64_t _maxSize;
    uint64_t _topVaddr;
    uint64_t _rsp;
    uint64_t _lowestVaddr;
    uint64_t _pageSize;

    void _checkPages();
    inline void _validateVaddr(uint64_t vaddr, uint64_t size);

   public:
    StackMemory(VirtualMemory& vmem, uint64_t topVaddr, uint64_t maxSize);
    ~StackMemory();

    void push(uint64_t val);
    uint64_t pop();

    void updateRsp(uint64_t rsp);  // called after each cpu cycle
    uint64_t rsp() const;
    uint64_t topVaddr() const;

    uint8_t read8(uint64_t vaddr);
    uint16_t read16(uint64_t vaddr);
    uint32_t read32(uint64_t vaddr);
    uint64_t read64(uint64_t vaddr);

    void write8(uint64_t vaddr, uint8_t value);
    void write16(uint64_t vaddr, uint16_t value);
    void write32(uint64_t vaddr, uint32_t value);
    void write64(uint64_t vaddr, uint64_t value);

    StackMemory(const StackMemory&) = delete;
    StackMemory& operator=(const StackMemory&) = delete;
    StackMemory(StackMemory&&) = delete;
    StackMemory& operator=(StackMemory&&) = delete;
};