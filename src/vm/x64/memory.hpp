#pragma once

#include <vector>

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

struct MemoryRegion {
    uint64_t base;
    uint64_t size;
    uint8_t perm = MEM_PERM_NONE;
    std::string name;
    uint64_t maxSize;
    bool growsUpward = true;  // false for stack, true otherwise
    uint64_t r1;  // internal, for efficient range checks (first address of address range)
    uint64_t r2;  // same as r1 (last address)
};

class Memory {
   private:
    VMState* _state = nullptr;
    ByteBuffer _ram;
    std::vector<MemoryRegion> _regions;
    MemoryRegion& _findRegion(uint64_t addr);

   public:
    Memory();
    // Memory(VMState* state);
    void init(VMState* state);

    // inserts the region into the list of regions, ensuring the correct order of r1
    void mapRegion(MemoryRegion rgn);

    uint8_t read8(uint64_t addr);
    uint16_t read16(uint64_t addr);
    uint32_t read32(uint64_t addr);
    uint64_t read64(uint64_t addr);
    ByteBuffer read(uint64_t addr, uint64_t len);

    void write8(uint64_t addr, uint8_t value);
    void write16(uint64_t addr, uint16_t value);
    void write32(uint64_t addr, uint32_t value);
    void write64(uint64_t addr, uint64_t value);
    void write(uint64_t addr, const ByteBuffer& data);

    // uint8_t fetch8(uint64_t addr);
    // uint16_t fetch16(uint64_t addr);
    // uint32_t fetch32(uint64_t addr);
    // uint64_t fetch64(uint64_t addr);

    Memory(const Memory& other) = delete;
    Memory& operator=(const Memory& other) = delete;
};