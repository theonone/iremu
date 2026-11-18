#include "vmem.hpp"

#include <iostream>

#include "../../common/errors.hpp"
#include "state.hpp"

// for the sake of performance, we'll assume we never fail to init() an instance before using it,
// even though that's not a very good idea in terms of safety

// TODO: review and rewrite this entire fucking bullshit class cuz it's ASS rn

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

// TODO: may rewrite to use a linked list-like approach:
// VPage will correspond to a SINGLE page, but store the address of the "next" page if multiple were
// allocated at once
// that way, resolving the base address of the needed page becomes an O(1) operation
// deletion starts at a page, then deletes all "next" pages until finds one where next=0
const VPage& VirtualMemory::_findVPage(uint64_t vaddr) const {
    auto it = _pageMap.upper_bound(vaddr);

    if (it == _pageMap.begin())
        throw MemoryError("No page allocated at " + std::to_string(vaddr));
    --it;

    const VPage& pg = it->second;
    if (vaddr - pg.baseVAddr >= pg.pages * _pageSize) {
        for (const auto& p : _pageMap) {
            std::cout << p.first << " - " << p.second.pages << std::endl;
        }
        throw MemoryError("No page allocated at " + std::to_string(vaddr));
    }

    return pg;
}

VirtualMemory::VirtualMemory() {}

VirtualMemory::~VirtualMemory() {
    for (const auto& [vaddr, vpage] : _pageMap) {
        for (uint64_t i = 0; i < vpage.pages; ++i) {
            _state->ram.free(vpage.basePAddr + (_pageSize * i));
        }
    }
}

void VirtualMemory::init(VMState* state) {
    _state = state;
    _pageSize = state->config.page_size;
    _p2 = _findPower2();
    _minVAddr = _pageSize;  // placeholder

    // for the sake of efficiency. with this assumption in place, we can be sure that a single
    // read/write never spills out of 2 pages.
    if (_pageSize < 8) {
        throw InputError("page_size must be at least 8B");
    }
}

int64_t VirtualMemory::allocate(uint64_t bytes, uint8_t perm, int64_t vaddr) {
    if (vaddr >= 0) {
        auto padding = vaddr - _firstPgAddr(vaddr);  // padding

        if (bytes > UINT64_MAX - padding) {  // I do NOT think achieving overflow here is a
                                             // realistic case but whatever
            return -1;
        }

        bytes += padding;
    }
    auto pages = (bytes / _pageSize) + (bytes % _pageSize != 0);
    uint64_t pageBase = _minVAddr;
    uint64_t sizeToAlloc = pages * _pageSize;

    if (vaddr >= 0) {
        if (vaddr < _minVAddr) {
            return -1;  // cannot allocate there
        }
        pageBase = _firstPgAddr(vaddr);

        uint64_t last = 0;
        // TODO: rewrite it properly
        for (const auto& p : _pageMap) {
            if (p.first < pageBase) {
                last = p.first + (p.second.pages * _pageSize);
                continue;
            }

            if ((p.first - pageBase) < sizeToAlloc) {
                return -1;
            } else {
                break;
            }
        }

        if (last >= vaddr) {
            return -1;  // already belongs to an allocation
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
    if (sizeToAlloc > UINT64_MAX - pageBase)
        return -1;

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

    _pageMap.erase(it);
}

uint8_t VirtualMemory::read8(uint64_t vaddr) {
    auto pg = _findVPage(vaddr);
    if (!(pg.perm & MEM_PERM_R)) {
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    }
    uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
    return _state->ram.read8(paddr);  // a single byte never spills into a different page
}

// if we allocate N pages of vram at once via one allocate() call, then all N pages will be adjacent
// in the physical memory
uint16_t VirtualMemory::read16(uint64_t vaddr) {
    auto pg = _findVPage(vaddr);
    if (!(pg.perm & MEM_PERM_R)) {
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
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
    if (!(pg2.perm & MEM_PERM_R)) {
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    }
    uint64_t paddr2 = pg2.basePAddr + ((vaddr + 1) - pg2.baseVAddr);
    uint16_t b2 = _state->ram.read8(paddr2) << 8;
    return b1 | b2;
}
uint32_t VirtualMemory::read32(uint64_t vaddr) {
    auto pg = _findVPage(vaddr);
    if (!(pg.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 4)
        return _state->ram.read32(paddr);

    uint32_t result = 0;

    for (uint64_t i = 0; i < remaining; ++i)
        result |= uint32_t(_state->ram.read8(paddr + i)) << (i * 8);

    auto pg2 = _findVPage(vaddr + remaining);

    if (!(pg2.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 4; ++i)
        result |= uint32_t(_state->ram.read8(paddr2 + i - remaining)) << (i * 8);

    return result;
}

uint64_t VirtualMemory::read64(uint64_t vaddr) {
    auto pg = _findVPage(vaddr);

    if (!(pg.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 8)
        return _state->ram.read64(paddr);

    uint64_t result = 0;

    for (uint64_t i = 0; i < remaining; ++i)
        result |= uint64_t(_state->ram.read8(paddr + i)) << (i * 8);

    auto pg2 = _findVPage(vaddr + remaining);

    if (!(pg2.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 8; ++i)
        result |= uint64_t(_state->ram.read8(paddr2 + i - remaining)) << (i * 8);

    return result;
}
ByteBuffer VirtualMemory::read(uint64_t vaddr, uint64_t len) {
    ByteBuffer result;
    result.reserve(len);

    uint64_t copied = 0;

    while (copied < len) {
        auto pg = _findVPage(vaddr + copied);

        if (!(pg.perm & MEM_PERM_R))
            throw MemoryError("Cannot read from " + std::to_string(vaddr + copied) +
                              " - reading not permitted at the corresponding page");

        uint64_t offset = vaddr + copied - pg.baseVAddr;
        uint64_t available = pg.pages * _pageSize - offset;
        uint64_t chunk = std::min(available, len - copied);

        uint64_t paddr = pg.basePAddr + offset;

        ByteBuffer part = _state->ram.read(paddr, chunk);
        result += part;

        copied += chunk;
    }

    return result;
}

void VirtualMemory::write8(uint64_t vaddr, uint8_t value) {
    auto pg = _findVPage(vaddr);

    if (!(pg.perm & MEM_PERM_W)) {
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");
    }

    uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
    _state->ram.write8(paddr, value);
}

void VirtualMemory::write16(uint64_t vaddr, uint16_t value) {
    auto pg = _findVPage(vaddr);

    if (!(pg.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 2) {
        _state->ram.write16(paddr, value);
        return;
    }

    _state->ram.write8(paddr, value & 0xFF);

    auto pg2 = _findVPage(vaddr + 1);
    if (!(pg2.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr + 1) +
                          " - writing not permitted at the corresponding page");

    uint64_t paddr2 = pg2.basePAddr + (vaddr + 1 - pg2.baseVAddr);
    _state->ram.write8(paddr2, (value >> 8) & 0xFF);
}

void VirtualMemory::write32(uint64_t vaddr, uint32_t value) {
    auto pg = _findVPage(vaddr);

    if (!(pg.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 4) {
        _state->ram.write32(paddr, value);
        return;
    }

    for (uint64_t i = 0; i < remaining; ++i)
        _state->ram.write8(paddr + i, (value >> (i * 8)) & 0xFF);

    auto pg2 = _findVPage(vaddr + remaining);
    if (!(pg2.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr + remaining) +
                          " - writing not permitted at the corresponding page");

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 4; ++i)
        _state->ram.write8(paddr2 + i - remaining, (value >> (i * 8)) & 0xFF);
}

void VirtualMemory::write64(uint64_t vaddr, uint64_t value) {
    auto pg = _findVPage(vaddr);

    if (!(pg.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 8) {
        _state->ram.write64(paddr, value);
        return;
    }

    for (uint64_t i = 0; i < remaining; ++i)
        _state->ram.write8(paddr + i, (value >> (i * 8)) & 0xFF);

    auto pg2 = _findVPage(vaddr + remaining);
    if (!(pg2.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr + remaining) +
                          " - writing not permitted at the corresponding page");

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 8; ++i)
        _state->ram.write8(paddr2 + i - remaining, (value >> (i * 8)) & 0xFF);
}

void VirtualMemory::write(uint64_t vaddr, const ByteBuffer& data) {
    uint64_t len = data.size();
    uint64_t copied = 0;

    while (copied < len) {
        auto pg = _findVPage(vaddr + copied);

        if (!(pg.perm & MEM_PERM_W))
            throw MemoryError("Cannot write to " + std::to_string(vaddr + copied) +
                              " - writing not permitted at the corresponding page");

        uint64_t offset = vaddr + copied - pg.baseVAddr;
        uint64_t available = pg.pages * _pageSize - offset;
        uint64_t chunk = std::min(available, len - copied);

        uint64_t paddr = pg.basePAddr + offset;

        ByteBuffer part = data.slice(copied, chunk);
        _state->ram.write(paddr, part);

        copied += chunk;
    }
}