#include "memory.hpp"

#include <cstring>

#include "../../common/errors.hpp"
#include "state.hpp"

MemoryRegion& Memory::_findRegion(size_t addr) {
    // regions are sorted by r1
    for (auto& r : _regions) {
        if (addr >= r.r1 && addr <= r.r2) {
            return r;
        }
    }
    throw MemoryError("Address " + std::to_string(addr) + " corresponds to unmapped memory");
}

Memory::Memory() {
    if constexpr (std::endian::native != std::endian::little) {
        throw NotImplementedError("Non-little endian architectures are not yet supported");
    }
}

void Memory::init(VMState* state) {
    _state = state;
    _ram.resize(state->config.ram_size);
}

bool overlap(MemoryRegion& reg1, MemoryRegion& reg2) {
    return ((reg2.r1 <= reg1.r2) && (reg1.r1 <= reg2.r2));
}

// Memory::Memory(VMState* state) : _state(state) { _ram.resize(state->config.ram_size); }

void Memory::mapRegion(MemoryRegion rgn) {
    if (_state == nullptr) {
        throw RuntimeError("Iremu bug: Memory object not initialized");
    }
    auto it = _regions.begin();
    for (; it != _regions.end(); ++it) {
        if ((*it).base >= rgn.base)
            break;
    }

    if (rgn.growsUpward) {
        rgn.r1 = rgn.base;
        rgn.r2 = rgn.base + rgn.size - 1;
    } else {
        rgn.r1 = rgn.base - rgn.size + 1;
        rgn.r2 = rgn.base;
    }

    if (rgn.r2 + 1 > _ram.size()) {
        throw MemoryError("Trying to map a region past RAM's last address (last addr=" +
                          std::to_string(rgn.r2 + 1) + ", RAM size=" + std::to_string(_ram.size()) +
                          ")");
    }

    // validity checks
    auto inserted = _regions.insert(it, rgn);
    auto next = inserted + 1;
    if (next != _regions.end()) {
        if (overlap(*next, *inserted)) {
            throw MemoryError("Bad memory mapping: address spaces overlap (base=" +
                              std::to_string(inserted->base) +
                              " and base=" + std::to_string(next->base) + ")");
        }
    }
    if (inserted != _regions.begin()) {
        auto prev = inserted - 1;
        if (overlap(*prev, *inserted)) {
            throw MemoryError("Bad memory mapping: address spaces overlap (base " +
                              std::to_string(inserted->base) + " and base " +
                              std::to_string(prev->base) + ")");
        }
    }
}

uint8_t Memory::read8(size_t addr) {
    auto& region = _findRegion(addr);
    if (region.perm & MEM_PERM_R) {
        return _ram[addr];
    }
    throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow reading");
}

uint16_t Memory::read16(size_t addr) {
    auto& region = _findRegion(addr);
    if (addr + 1 > region.r2) {
        throw MemoryError("Read of 2 bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }
    if (region.perm & MEM_PERM_R) {
        uint16_t val;
        std::memcpy(&val, _ram.cdata() + addr, sizeof(uint16_t));
        return val;
    }
    throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow reading");
}
uint32_t Memory::read32(size_t addr) {
    auto& region = _findRegion(addr);
    if (addr + 3 > region.r2) {
        throw MemoryError("Read of 4 bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }
    if (region.perm & MEM_PERM_R) {
        uint32_t val;
        std::memcpy(&val, _ram.cdata() + addr, sizeof(uint32_t));
        return val;
    }
    throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow reading");
}
uint64_t Memory::read64(size_t addr) {
    auto& region = _findRegion(addr);
    if (addr + 7 > region.r2) {
        throw MemoryError("Read of 8 bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }
    if (region.perm & MEM_PERM_R) {
        uint64_t val;
        std::memcpy(&val, _ram.cdata() + addr, sizeof(uint64_t));
        return val;
    }
    throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow reading");
}
ByteBuffer Memory::read(size_t addr, size_t len) {
    auto& region = _findRegion(addr);
    if (addr + len > region.r2) {
        throw MemoryError("Read of " + std::to_string(len) + " bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }
    if (region.perm & MEM_PERM_R) {
        return _ram.slice(addr, addr + len);
    }
    throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow reading");
}

void Memory::write8(size_t addr, uint8_t value) {
    auto& region = _findRegion(addr);
    if (!(region.perm & MEM_PERM_W)) {
        throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow writing");
    }
    _ram[addr] = value;
}
void Memory::write16(size_t addr, uint16_t value) {
    auto& region = _findRegion(addr);
    if (addr + 1 > region.r2) {
        throw MemoryError("Write of 2 bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }
    if (!(region.perm & MEM_PERM_W)) {
        throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow writing");
    }
    std::memcpy(_ram.data() + addr, &value, sizeof(uint16_t));
}
void Memory::write32(size_t addr, uint32_t value) {
    auto& region = _findRegion(addr);
    if (addr + 3 > region.r2) {
        throw MemoryError("Write of 4 bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }
    if (!(region.perm & MEM_PERM_W)) {
        throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow writing");
    }
    std::memcpy(_ram.data() + addr, &value, sizeof(uint32_t));
}
void Memory::write64(size_t addr, uint64_t value) {
    auto& region = _findRegion(addr);
    if (addr + 7 > region.r2) {
        throw MemoryError("Write of 8 bytes at " + std::to_string(addr) +
                          " spills out of the region");
    }

    if (!(region.perm & MEM_PERM_W)) {
        throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow writing");
    }
    std::memcpy(_ram.data() + addr, &value, sizeof(uint64_t));
}
void Memory::write(size_t addr, const ByteBuffer& data) {
    auto& region = _findRegion(addr);
    if (addr + data.size() > region.r2) {
        throw MemoryError("Write of " + std::to_string(data.size()) + " bytes at " +
                          std::to_string(addr) + " spills out of the region");
    }
    if (region.perm & MEM_PERM_W) {
        std::memcpy(_ram.data() + addr, data.cdata(), data.size());
    }
    throw MemoryError("Memory region at " + std::to_string(addr) + " does not allow writing");
}
