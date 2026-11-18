#pragma once

#include <map>

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

struct VAlloc {
    uint64_t basePAddr;            // lowest address of the allocation in physical memory
    uint64_t baseVAddr;            // convenient to have, even though it's the key in _pageMap
    uint64_t pages;                // page count
    uint8_t perm = MEM_PERM_NONE;  // bit mask of permissions
};

class VirtualMemory {
   private:
    VMState* _state = nullptr;
    std::map<uint64_t, VAlloc> _pageMap;  // vAddr -> page. TODO: think how to use an unordered_map
    uint64_t _pageSize;                   // VAlloc size must match pPage size
    uint8_t _p2;                          // for bit shifting instead of division
    inline uint64_t _firstPgAddr(
        uint64_t vaddr) const;  // returns the lowest virtual address of the page
    uint64_t _minVAddr;

    uint8_t _findPower2();
    const VAlloc& _findVAlloc(uint64_t vaddr) const;

   public:
    VirtualMemory();
    ~VirtualMemory();

    void init(VMState* state);

    // allocates at least $bytes bytes starting somewhere
    uint64_t allocate(uint64_t bytes, uint8_t perm);

    // allocates at least $bytes bytes starting at $vaddr
    uint64_t allocate(uint64_t bytes, uint64_t vaddr, uint8_t perm);

    void free(uint64_t vaddr);

    uint8_t read8(uint64_t vaddr);
    uint16_t read16(uint64_t vaddr);
    uint32_t read32(uint64_t vaddr);
    uint64_t read64(uint64_t vaddr);
    ByteBuffer read(uint64_t vaddr, uint64_t len);

    // fetch methods - TODO

    void write8(uint64_t vaddr, uint8_t value);
    void write16(uint64_t vaddr, uint16_t value);
    void write32(uint64_t vaddr, uint32_t value);
    void write64(uint64_t vaddr, uint64_t value);
    void write(uint64_t vaddr, const ByteBuffer& data);

    const std::map<uint64_t, VAlloc>& pageMap() const;

    VirtualMemory(const VirtualMemory&) = delete;
    VirtualMemory& operator=(const VirtualMemory&) = delete;
};