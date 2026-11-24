#include "vmem.hpp"

#include <iostream>

#include "../../common/errors.hpp"
#include "../cfgProv.hpp"
#include "vmState.hpp"

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
// VAlloc will correspond to a SINGLE page, but store the address of the "next" page if multiple
// were allocated at once that way, resolving the base address of the needed page becomes an O(1)
// operation deletion starts at a page, then deletes all "next" pages until finds one where next=0
const VAlloc& VirtualMemory::_findVAlloc(uint64_t vaddr) const {
    auto it = _pageMap.upper_bound(vaddr);

    if (it == _pageMap.begin())
        throw MemoryError("No page allocated at " + std::to_string(vaddr));
    --it;

    const VAlloc& pg = it->second;
    if (vaddr - pg.baseVAddr >= pg.pages * _pageSize) {
        std::cout << _pageMap.size() << " pages allocated:" << std::endl;
        for (const auto& p : _pageMap) {
            std::cout << p.first << " - " << p.second.pages << std::endl;
        }
        throw MemoryError("No page allocated at " + std::to_string(vaddr));
    }

    return pg;
}

VirtualMemory::VirtualMemory() {}

VirtualMemory::~VirtualMemory() {
    for (const auto& [vaddr, vAlloc] : _pageMap) {
        for (uint64_t i = 0; i < vAlloc.pages; ++i) {
            _vmState->ram.free(vAlloc.basePAddr + (_pageSize * i));
        }
    }
}

void VirtualMemory::init(VMState* vmState) {
    _vmState = vmState;
    _pageSize = ConfigProvider::get().page_size;
    _p2 = _findPower2();
    _minVAddr = _pageSize;  // placeholder

    // for the sake of efficiency. with this assumption in place, we can be sure that a single
    // read/write never spills out of 2 pages.
    if (_pageSize < 8) {
        throw InputError("page_size must be at least 8B");
    }
}

uint64_t VirtualMemory::allocate(uint64_t bytes, uint8_t perm) {
    auto pages = (bytes / _pageSize) + (bytes % _pageSize != 0);
    uint64_t pageBase = _minVAddr;
    // if (uint64_max < pages*pageSize)
    if (UINT64_MAX / pages < _pageSize) {
        throw MemoryError("Allocation too large");
    }
    uint64_t sizeToAlloc = pages * _pageSize;

    bool found = false;
    for (const auto& p : _pageMap) {
        if ((p.first - pageBase) >= sizeToAlloc) {
            found = true;
            break;
        }
        pageBase = p.first + (p.second.pages * _pageSize);
    }

    if (!found && ((pageBase > UINT64_MAX - sizeToAlloc))) {
        throw MemoryError("Could not find space to allocate " + std::to_string(bytes) + " bytes");
    }

    uint64_t paddr = _vmState->ram.allocate(pages);

    // atp we've allocated $pages pages starting at $addr
    _pageMap[pageBase] = {.basePAddr = paddr, .baseVAddr = pageBase, .pages = pages, .perm = perm};
    return pageBase;
}

// uint64_t VirtualMemory::allocate(uint64_t bytes, uint64_t vaddr, uint8_t perm) {
//     auto padding = vaddr - _firstPgAddr(vaddr);  // padding

//     // bytes + padding > UINT64_MAX
//     if (bytes > UINT64_MAX - padding) {  // I do NOT think achieving overflow here is a
//                                          // realistic case but whatever
//         throw MemoryError("Allocation exceeds the address limit");
//     }

//     bytes += padding;

//     auto pages = (bytes / _pageSize) + (bytes % _pageSize != 0);
//     uint64_t pageBase = _minVAddr;
//     uint64_t sizeToAlloc = pages * _pageSize;
//     if (UINT64_MAX / pages < _pageSize) {
//         throw MemoryError("Allocation too large");
//     }

//     if (vaddr < _minVAddr) {
//         throw MemoryError("Cannot allocate at " + std::to_string(vaddr) +
//                           ": the minimal available virtual address is " +
//                           std::to_string(_minVAddr));
//     }
//     pageBase = _firstPgAddr(vaddr);

//     uint64_t last = 0;
//     // TODO: rewrite it properly
//     for (const auto& p : _pageMap) {
//         if (p.first < pageBase) {
//             last = p.first + (p.second.pages * _pageSize);
//             continue;
//         }

//         if ((p.first - pageBase) < sizeToAlloc) {
//             throw MemoryError("Cannot allocate at the address as it's already taken");

//         } else {
//             break;
//         }
//     }

//     if (last >= vaddr) {
//         throw MemoryError("Cannot allocate at the address as it's already taken");
//     }
//     if (sizeToAlloc > UINT64_MAX - pageBase)
//         throw MemoryError("Allocation exceeds the address limit");

//     uint64_t paddr = _vmState->ram.allocate(pages);

//     // atp we've allocated $pages pages starting at $addr
//     _pageMap[pageBase] = {.basePAddr = paddr, .baseVAddr = pageBase, .pages = pages, .perm =
//     perm}; return pageBase;
// }

uint64_t VirtualMemory::allocate(uint64_t bytes, uint64_t vaddr, uint8_t perm) {
    if (vaddr < _minVAddr) {
        throw MemoryError("Cannot allocate at " + std::to_string(vaddr) +
                          ": the minimal available virtual address is " +
                          std::to_string(_minVAddr));
    }

    uint64_t pageBase = _firstPgAddr(vaddr);
    uint64_t padding = vaddr - pageBase;

    if (bytes > UINT64_MAX - padding) {
        throw MemoryError("Allocation exceeds the address limit");
    }

    bytes += padding;

    uint64_t pages = (bytes / _pageSize) + (bytes % _pageSize != 0);

    if (pages > UINT64_MAX / _pageSize) {
        throw MemoryError("Allocation exceeds the address limit");
    }

    uint64_t sizeToAlloc = pages * _pageSize;

    if (sizeToAlloc > UINT64_MAX - pageBase) {
        throw MemoryError("Allocation exceeds the address limit");
    }

    // first allocation that starts after vaddr
    auto next = _pageMap.lower_bound(pageBase);

    if (next != _pageMap.end() && next->first - pageBase < sizeToAlloc) {
        throw MemoryError(
            "Cannot allocate: virtual address range is already occupied (next page starts at " +
            std::to_string(next->first) + ", need " + std::to_string(sizeToAlloc) + " bytes from " +
            std::to_string(pageBase) + ")");
    }

    if (next != _pageMap.begin()) {
        auto prev = --next;
        uint64_t prevEnd = prev->first + (prev->second.pages * _pageSize);

        if (prevEnd > pageBase) {
            throw MemoryError("Cannot allocate: virtual address range is already occupied");
        }
    }

    uint64_t paddr = _vmState->ram.allocate(pages);

    _pageMap[pageBase] = {.basePAddr = paddr, .baseVAddr = pageBase, .pages = pages, .perm = perm};

    return pageBase;
}

void VirtualMemory::free(uint64_t vaddr) {
    auto lPageAddr = _firstPgAddr(vaddr);
    auto it = _pageMap.find(lPageAddr);
    if (it == _pageMap.end()) {
        return;
        // throw MemoryError("Cannot free memory at " + std::to_string(vaddr) +
        //                   " - no page is allocated there");
    }
    for (uint64_t i = 0; i < it->second.pages; ++i) {
        _vmState->ram.free(it->second.basePAddr + (_pageSize * i));
    }

    _pageMap.erase(it);
}

// void VirtualMemory::addPermissionsRange(uint64_t vaddr1, uint64_t vaddr2, uint8_t perm) {
//     auto it = _pageMap.lower_bound(vaddr1);

//     for (; it != _pageMap.end(); ++it) {
//         auto& entry = *it;
//         if (entry.first > vaddr2) {
//             break;
//         }
//         entry.second.perm |= perm;
//     }
// }

uint8_t VirtualMemory::read8(uint64_t vaddr) {
    auto pg = _findVAlloc(vaddr);
    if (!(pg.perm & MEM_PERM_R)) {
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    }
    uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
    return _vmState->ram.read8(paddr);  // a single byte never spills into a different page
}

// if we allocate N pages of vram at once via one allocate() call, then all N pages will be adjacent
// in the physical memory
uint16_t VirtualMemory::read16(uint64_t vaddr) {
    auto pg = _findVAlloc(vaddr);
    if (!(pg.perm & MEM_PERM_R)) {
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    }

    // if the last addr of the read is less or equal to the last address of the allocation, then it
    // doesn't spill into a different allocation
    if ((pg.baseVAddr + _pageSize * pg.pages) >= (vaddr + 2)) {
        uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
        return _vmState->ram.read16(paddr);
    }

    // spilling case
    uint64_t paddr1 = pg.basePAddr + (vaddr - pg.baseVAddr);
    uint8_t b1 = _vmState->ram.read8(paddr1);

    auto pg2 = _findVAlloc(vaddr + 1);
    if (!(pg2.perm & MEM_PERM_R)) {
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    }
    uint64_t paddr2 = pg2.basePAddr + ((vaddr + 1) - pg2.baseVAddr);
    uint16_t b2 = _vmState->ram.read8(paddr2) << 8;
    return b1 | b2;
}
uint32_t VirtualMemory::read32(uint64_t vaddr) {
    auto pg = _findVAlloc(vaddr);
    if (!(pg.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 4)
        return _vmState->ram.read32(paddr);

    uint32_t result = 0;

    for (uint64_t i = 0; i < remaining; ++i)
        result |= uint32_t(_vmState->ram.read8(paddr + i)) << (i * 8);

    auto pg2 = _findVAlloc(vaddr + remaining);

    if (!(pg2.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 4; ++i)
        result |= uint32_t(_vmState->ram.read8(paddr2 + i - remaining)) << (i * 8);

    return result;
}

uint64_t VirtualMemory::read64(uint64_t vaddr) {
    auto pg = _findVAlloc(vaddr);

    if (!(pg.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));
    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 8)
        return _vmState->ram.read64(paddr);

    uint64_t result = 0;

    for (uint64_t i = 0; i < remaining; ++i)
        result |= uint64_t(_vmState->ram.read8(paddr + i)) << (i * 8);

    auto pg2 = _findVAlloc(vaddr + remaining);

    if (!(pg2.perm & MEM_PERM_R))
        throw MemoryError("Read permission denied at " + std::to_string(vaddr));

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 8; ++i)
        result |= uint64_t(_vmState->ram.read8(paddr2 + i - remaining)) << (i * 8);

    return result;
}
ByteBuffer VirtualMemory::read(uint64_t vaddr, uint64_t len) {
    ByteBuffer result;
    result.reserve(len);

    uint64_t copied = 0;

    while (copied < len) {
        auto pg = _findVAlloc(vaddr + copied);

        if (!(pg.perm & MEM_PERM_R))
            throw MemoryError("Cannot read from " + std::to_string(vaddr + copied) +
                              " - reading not permitted at the corresponding page");

        uint64_t offset = vaddr + copied - pg.baseVAddr;
        uint64_t available = pg.pages * _pageSize - offset;
        uint64_t chunk = std::min(available, len - copied);

        uint64_t paddr = pg.basePAddr + offset;

        ByteBuffer part = _vmState->ram.read(paddr, chunk);
        result += part;

        copied += chunk;
    }

    return result;
}

void VirtualMemory::write8(uint64_t vaddr, uint8_t value) {
    auto pg = _findVAlloc(vaddr);

    if (!(pg.perm & MEM_PERM_W)) {
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");
    }

    uint64_t paddr = pg.basePAddr + (vaddr - pg.baseVAddr);
    _vmState->ram.write8(paddr, value);
}

void VirtualMemory::write16(uint64_t vaddr, uint16_t value) {
    auto pg = _findVAlloc(vaddr);

    if (!(pg.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 2) {
        _vmState->ram.write16(paddr, value);
        return;
    }

    _vmState->ram.write8(paddr, value & 0xFF);

    auto pg2 = _findVAlloc(vaddr + 1);
    if (!(pg2.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr + 1) +
                          " - writing not permitted at the corresponding page");

    uint64_t paddr2 = pg2.basePAddr + (vaddr + 1 - pg2.baseVAddr);
    _vmState->ram.write8(paddr2, (value >> 8) & 0xFF);
}

void VirtualMemory::write32(uint64_t vaddr, uint32_t value) {
    auto pg = _findVAlloc(vaddr);

    if (!(pg.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 4) {
        _vmState->ram.write32(paddr, value);
        return;
    }

    for (uint64_t i = 0; i < remaining; ++i)
        _vmState->ram.write8(paddr + i, (value >> (i * 8)) & 0xFF);

    auto pg2 = _findVAlloc(vaddr + remaining);
    if (!(pg2.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr + remaining) +
                          " - writing not permitted at the corresponding page");

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 4; ++i)
        _vmState->ram.write8(paddr2 + i - remaining, (value >> (i * 8)) & 0xFF);
}

void VirtualMemory::write64(uint64_t vaddr, uint64_t value) {
    auto pg = _findVAlloc(vaddr);

    if (!(pg.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr) +
                          " - writing not permitted at the corresponding page");

    uint64_t offset = vaddr - pg.baseVAddr;
    uint64_t remaining = pg.pages * _pageSize - offset;
    uint64_t paddr = pg.basePAddr + offset;

    if (remaining >= 8) {
        _vmState->ram.write64(paddr, value);
        return;
    }

    for (uint64_t i = 0; i < remaining; ++i)
        _vmState->ram.write8(paddr + i, (value >> (i * 8)) & 0xFF);

    auto pg2 = _findVAlloc(vaddr + remaining);
    if (!(pg2.perm & MEM_PERM_W))
        throw MemoryError("Cannot write to " + std::to_string(vaddr + remaining) +
                          " - writing not permitted at the corresponding page");

    uint64_t paddr2 = pg2.basePAddr + (vaddr + remaining - pg2.baseVAddr);

    for (uint64_t i = remaining; i < 8; ++i)
        _vmState->ram.write8(paddr2 + i - remaining, (value >> (i * 8)) & 0xFF);
}

void VirtualMemory::write(uint64_t vaddr, const ByteBuffer& data, bool bypassPerm) {
    uint64_t len = data.size();
    uint64_t copied = 0;

    while (copied < len) {
        auto pg = _findVAlloc(vaddr + copied);

        if ((!bypassPerm) && !(pg.perm & MEM_PERM_W))
            throw MemoryError("Cannot write to " + std::to_string(vaddr + copied) +
                              " - writing not permitted at the corresponding page");

        uint64_t offset = vaddr + copied - pg.baseVAddr;
        uint64_t available = pg.pages * _pageSize - offset;
        uint64_t chunk = std::min(available, len - copied);

        uint64_t paddr = pg.basePAddr + offset;

        ByteBuffer part = data.slice(copied, chunk);
        _vmState->ram.write(paddr, part);

        copied += chunk;
    }
}

const std::map<uint64_t, VAlloc>& VirtualMemory::pageMap() const { return _pageMap; }
