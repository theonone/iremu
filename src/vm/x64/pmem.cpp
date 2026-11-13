#include "pmem.hpp"

#include <cstring>

#include "../../common/errors.hpp"
#include "state.hpp"

PhysicalMemory::PhysicalMemory() {
    if constexpr (std::endian::native != std::endian::little) {
        throw NotImplementedError("Non-little endian architectures are not yet supported");
    }
}

void PhysicalMemory::init(VMState* state) {
    auto rsz = state->config.ram_size;
    if (rsz % _pageSize != 0) {
        throw InputError("RAM size must be divisible by page size (" + std::to_string(rsz) + ",  " +
                         std::to_string(_pageSize) + ")");
    }
    _state = state;
    _pageSize = state->config.page_size;
    _ram.resize(rsz);
    _pageBitmap = std::vector<bool>(rsz / _pageSize, false);
}

int64_t PhysicalMemory::allocate(uint64_t bytes) {
    uint64_t wholePages = (bytes / _pageSize) + (bytes % _pageSize != 0);
    if (wholePages == 0) {
        throw MemoryError("Cannot allocate 0 pages");
    }
    uint64_t copy = wholePages;
    // need to find a gap at least $wholePages pages long
    for (size_t i = 0; i < _pageBitmap.size(); ++i) {
        if (!_pageBitmap[i]) {
            if ((--copy) == 0) {
                // mark pages as used
                for (size_t j = 0; j < wholePages; ++j) {
                    _pageBitmap[i - j] = true;
                }
                // return the first address of the range
                return (i - wholePages + 1) * _pageSize;
            }
        } else {
            copy = wholePages;
        }
    }
    return -1;
}

void PhysicalMemory::free(uint64_t addr) {
    // addr is supposed to be the first address of the page to free
    _pageBitmap[addr / _pageSize] = false;
}

uint8_t PhysicalMemory::read8(uint64_t addr) { return _ram[addr]; }

uint16_t PhysicalMemory::read16(uint64_t addr) {
    uint16_t val;
    std::memcpy(&val, _ram.cdata() + addr, sizeof(uint16_t));
    return val;
}
uint32_t PhysicalMemory::read32(uint64_t addr) {
    uint32_t val;
    std::memcpy(&val, _ram.cdata() + addr, sizeof(uint32_t));
    return val;
}
uint64_t PhysicalMemory::read64(uint64_t addr) {
    uint64_t val;
    std::memcpy(&val, _ram.cdata() + addr, sizeof(uint64_t));
    return val;
}
ByteBuffer PhysicalMemory::read(uint64_t addr, uint64_t len) {
    return _ram.slice(addr, addr + len);
}

void PhysicalMemory::write8(uint64_t addr, uint8_t value) { _ram[addr] = value; }
void PhysicalMemory::write16(uint64_t addr, uint16_t value) {
    std::memcpy(_ram.data() + addr, &value, sizeof(uint16_t));
}
void PhysicalMemory::write32(uint64_t addr, uint32_t value) {
    std::memcpy(_ram.data() + addr, &value, sizeof(uint32_t));
}
void PhysicalMemory::write64(uint64_t addr, uint64_t value) {
    std::memcpy(_ram.data() + addr, &value, sizeof(uint64_t));
}
void PhysicalMemory::write(uint64_t addr, const ByteBuffer& data) {
    std::memcpy(_ram.data() + addr, data.cdata(), data.size());
}
