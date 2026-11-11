#pragma once

#include <vector>

#include "../../common/bytebuff.hpp"

class VMState;

class PhysicalMemory {
   private:
    VMState* _state = nullptr;
    ByteBuffer _ram;

    // using u8 instead of bool, because bools take a byte instead of a single bit. an element here

    // // describes 8 pages, each individual bit - whether a page is allocated or not
    // std::vector<uint8_t> _pageBitmap;  // might replace it with a plain array
    // later

    // I'll just use bools for now to avoid extra work
    // _pageBitmap[i] = true if used
    std::vector<bool> _pageBitmap;

    uint64_t _pageSize = 4096;

   public:
    PhysicalMemory();
    void init(VMState* state);

    int64_t allocate(uint64_t bytes);  // int -> addr, -1 if can't allocate
    void free(uint64_t addr);

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

    PhysicalMemory(const PhysicalMemory& other) = delete;
    PhysicalMemory& operator=(const PhysicalMemory& other) = delete;
};