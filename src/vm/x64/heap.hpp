#pragma once

#include "../x64/vmem.hpp"

class HeapMemory {
   private:
    VirtualMemory* _vmem = nullptr;
    uint64_t _maxSize;
    uint64_t _base;
    uint64_t _brk;
    uint64_t _mappedEnd;  // last address of the last page
    uint64_t _pageSize;

    inline void _validateVaddr(uint64_t vaddr, uint64_t size);

   public:
    HeapMemory(VirtualMemory* vmem);
    ~HeapMemory();

    // ONLY meant to be called by Loader before any real work happens
    void setBase(uint64_t base);

    int brk(uint64_t vaddr);
    uint64_t sbrk(int64_t bytes);

    uint64_t mappedEnd() const;
    uint64_t base() const;

    uint8_t read8(uint64_t vaddr);
    uint16_t read16(uint64_t vaddr);
    uint32_t read32(uint64_t vaddr);
    uint64_t read64(uint64_t vaddr);

    void write8(uint64_t vaddr, uint8_t value);
    void write16(uint64_t vaddr, uint16_t value);
    void write32(uint64_t vaddr, uint32_t value);
    void write64(uint64_t vaddr, uint64_t value);

    HeapMemory(const HeapMemory&) = delete;
    HeapMemory& operator=(const HeapMemory&) = delete;
    HeapMemory(HeapMemory&&) = delete;
    HeapMemory& operator=(HeapMemory&&) = delete;
};