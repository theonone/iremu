#include "vmem.hpp"

#include "../../common/errors.hpp"
#include "state.hpp"

// for the sake of performance, we'll assume we never fail to init() an instance before using it,
// even though that's not a very good idea in terms of safety

// equivalent to (vaddr / _pageSize) * _pageSize
inline uint64_t VirtualMemory::_firstPgAddr(uint64_t vaddr) const { return (vaddr >> _p2) << _p2; }

uint8_t VirtualMemory::_findPower2() {
    for (uint8_t i = 0; i < 64; ++i) {
        if ((uint64_t(1) << i) == _pageSize) {
            return i;
        }
    }
    throw MemoryError("page_size in bytes must be a power of 2");
}

const VPage& VirtualMemory::_findVPage(uint64_t vaddr) const {
    auto it = _pageMap.upper_bound(vaddr);

    if (it == _pageMap.begin())
        throw MemoryError("No page allocated at " + std::to_string(vaddr));
    --it;

    const VPage& pg = it->second;
    if (vaddr - pg.baseVAddr >= pg.pages * _pageSize)
        throw MemoryError("No page allocated at " + std::to_string(vaddr));

    return pg;
}

VirtualMemory::VirtualMemory() {}

void VirtualMemory::init(VMState* state) {
    _state = state;
    _pageSize = state->config.page_size;
    _p2 = _findPower2();
}

int64_t VirtualMemory::allocate(uint64_t bytes, uint8_t perm, int64_t vaddr) {
    auto pages = (bytes / _pageSize) + (bytes % _pageSize != 0);
    uint64_t pageBase = 0;
    uint64_t sizeToAlloc = pages * _pageSize;

    if (vaddr >= 0) {
        pageBase = _firstPgAddr(vaddr);

        for (const auto& p : _pageMap) {
            if (p.first < pageBase) {
                continue;
            }
            if ((p.first - pageBase) < sizeToAlloc) {
                return -1;
            }
        }
    } else {
        bool found = false;
        for (const auto& p : _pageMap) {
            if ((p.first - pageBase) >= sizeToAlloc) {
                found = true;
                break;
            }
            pageBase = p.first + (p.second.pages * _pageSize);
        }

        // the second condition will basically never be true, so might as well just remove it
        if (!found && ((pageBase + sizeToAlloc > INT64_MAX))) {
            return -1;
        }
    }
    if (pageBase + bytes < bytes) {  // if overflows
        return -1;
    }

    int64_t paddr = _state->ram.allocate(pages);
    if (paddr == -1) {
        return -1;
    }
    // atp we've allocated $pages pages starting at $addr
    _pageMap[pageBase] = {.basePAddr = paddr, .baseVAddr = pageBase, .pages = pages, .perm = perm};
    return pageBase;
}

void VirtualMemory::free(uint64_t vaddr) {
    auto lPageAddr = _firstPgAddr(vaddr);
    auto it = _pageMap.find(lPageAddr);
    if (it == _pageMap.end()) {
        throw MemoryError("Cannot free memory at " + std::to_string(vaddr) +
                          " - no page is allocated there");
    }
    for (uint64_t i = 0; i < it->second.pages; ++i) {
        _state->ram.free(it->second.basePAddr + (_pageSize * i));
    }
    _pageMap.erase(lPageAddr);
}

uint8_t VirtualMemory::read8(uint64_t vaddr) {
    auto pg = _findVPage(vaddr);
    if (!(pg.perm & MEM_PERM_R)) {
        throw MemoryError("Cannot read from " + std::to_string(vaddr) +
                          " - reading not permitted at the corresponding page");
    }
    uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
    return _state->ram.read8(paddr);  // a single byte never spills into a different page
}

// if we allocate N pages of vram at once via one allocate() call, then all N pages will be adjacent
// in the physical memory
uint16_t VirtualMemory::read16(uint64_t vaddr) {
    auto pg = _findVPage(vaddr);
    if (!(pg.perm & MEM_PERM_R)) {
        throw MemoryError("Cannot read from " + std::to_string(vaddr) +
                          " - reading not permitted at the corresponding page");
    }

    // if the last addr of the read is less or equal to the last address of the allocation, then it
    // doesn't spill into a different allocation
    if ((pg.baseVAddr + _pageSize * pg.pages) >= (vaddr + 2)) {
        uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
        return _state->ram.read16(paddr);
    }

    // spilling case
    uint64_t paddr1 = pg.basePAddr + (vaddr - pg.baseVAddr);
    uint8_t b1 = _state->ram.read8(paddr1);

    auto pg2 = _findVPage(vaddr + 1);
    uint64_t paddr2 = pg2.basePAddr + ((vaddr + 1) - pg2.baseVAddr);
    uint16_t b2 = _state->ram.read8(paddr2) << 8;
    return b1 | b2;
}
uint32_t VirtualMemory::read32(uint64_t vaddr) { return 0; }
uint64_t VirtualMemory::read64(uint64_t vaddr) { return 0; }
ByteBuffer VirtualMemory::read(uint64_t vaddr, uint64_t len) { return 0; }

void VirtualMemory::write8(uint64_t vaddr, uint8_t value) {}
void VirtualMemory::write16(uint64_t vaddr, uint16_t value) {}
void VirtualMemory::write32(uint64_t vaddr, uint32_t value) {}
void VirtualMemory::write64(uint64_t vaddr, uint64_t value) {}
void VirtualMemory::write(uint64_t vaddr, const ByteBuffer& data) {}
