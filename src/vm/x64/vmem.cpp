#include "vmem.hpp"

#include <math.h>

#include "../../common/errors.hpp"
#include "state.hpp"

// for the sake of performance, we'll assume we never fail to init() an instance before using it,
// even though that's not a very good idea in terms of safety

VirtualMemory::VirtualMemory() {}

void VirtualMemory::init(VMState* state) {
    _state = state;
    _pageSize = state->config.page_size;
    if (((uint64_t)(pow(2, 64) - 1) % (_pageSize)) != 0) {
        throw MemoryError("2^64-1 must be divisible by page_size");
    }
}

int64_t VirtualMemory::allocate(uint64_t bytes, uint8_t perm, int64_t vaddr) {
    int64_t addr = _state->ram.allocate(bytes);
    if (addr == -1) {
        return -1;
    }
    auto pages = (bytes / _pageSize) + (bytes % _pageSize != 0);
    // atp we've allocated $pages pages starting at $addr
    _pageMap[addr] = {.basePAddr = addr, .pages = pages, .perm = perm};
    return -1;
}

void VirtualMemory::free(uint64_t addr) {}

uint8_t VirtualMemory::read8(uint64_t addr) { return 0; }
uint16_t VirtualMemory::read16(uint64_t addr) { return 0; }
uint32_t VirtualMemory::read32(uint64_t addr) { return 0; }
uint64_t VirtualMemory::read64(uint64_t addr) { return 0; }
ByteBuffer VirtualMemory::read(uint64_t addr, uint64_t len) { return 0; }

void VirtualMemory::write8(uint64_t addr, uint8_t value) {}
void VirtualMemory::write16(uint64_t addr, uint16_t value) {}
void VirtualMemory::write32(uint64_t addr, uint32_t value) {}
void VirtualMemory::write64(uint64_t addr, uint64_t value) {}
void VirtualMemory::write(uint64_t addr, const ByteBuffer& data) {}
