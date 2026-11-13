#pragma once

#include <unordered_map>

#include "../../common/bytebuff.hpp"

constexpr uint8_t MEM_PERM_NONE = 0;
constexpr uint8_t MEM_PERM_R = 1;
constexpr uint8_t MEM_PERM_W = 1 << 1;
constexpr uint8_t MEM_PERM_X = 1 << 2;
constexpr uint8_t MEM_PERM_RW = MEM_PERM_R | MEM_PERM_W;
constexpr uint8_t MEM_PERM_RX = MEM_PERM_R | MEM_PERM_X;
constexpr uint8_t MEM_PERM_RWX = MEM_PERM_R | MEM_PERM_W | MEM_PERM_X;
constexpr uint8_t MEM_PERM_WX = MEM_PERM_W | MEM_PERM_X;

class VMState;

struct VPageAlloc {
    uint64_t basePAddr;            // lowest address of the allocation
    uint64_t pages;                // page count
    uint8_t perm = MEM_PERM_NONE;  // bit mask of permissions
};

class VirtualMemory {
   private:
    VMState* _state = nullptr;
    std::unordered_map<uint64_t, VPageAlloc> _pageMap;  // vAddr -> page
    uint64_t _pageSize;                                 // vPage size must match pPage size

   public:
    VirtualMemory();
    void init(VMState* state);

    int64_t allocate(uint64_t bytes, uint8_t perm = MEM_PERM_NONE,
                     int64_t vaddr = -1);  // int -> addr, -1 if can't allocate
    void free(uint64_t addr);

    uint8_t read8(uint64_t addr);
    uint16_t read16(uint64_t addr);
    uint32_t read32(uint64_t addr);
    uint64_t read64(uint64_t addr);
    ByteBuffer read(uint64_t addr, uint64_t len);

    // fetch methods - TODO

    void write8(uint64_t addr, uint8_t value);
    void write16(uint64_t addr, uint16_t value);
    void write32(uint64_t addr, uint32_t value);
    void write64(uint64_t addr, uint64_t value);
    void write(uint64_t addr, const ByteBuffer& data);

    VirtualMemory(const VirtualMemory&) = delete;
    VirtualMemory& operator=(const VirtualMemory&) = delete;
};