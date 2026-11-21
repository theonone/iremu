#include "stack.hpp"

#include "../../common/errors.hpp"
#include "../cfgProv.hpp"

void StackMemory::_checkPages() {
    if (_rsp - REDZONE < _lowestAddr) {
        uint64_t size = (_lowestAddr + REDZONE) - _rsp;
        uint64_t pages = size / _pageSize + (size % _pageSize != 0);
        _lowestAddr -= (pages * _pageSize);
        if (STACK_TOP - _lowestAddr > _maxSize) {
            throw MemoryError("Stack overflow. The limit of " + std::to_string(_maxSize) +
                              " was exceeded.");
        }
        _vmem.allocate(size, _lowestAddr, MEM_PERM_RW);
    }
}

inline void StackMemory::_validateVaddr(uint64_t vaddr, uint64_t size) {
    if ((vaddr < _rsp - REDZONE) || (vaddr > STACK_TOP - size)) {
        throw MemoryError("Bad stack memory access");
    }
}

StackMemory::StackMemory(VirtualMemory& vmem) : _vmem(vmem) {
    _maxSize = ConfigProvider::get()
                   .max_stack;  // if -1 i.e. unlimited, the value will turn into UINT64_MAX after
                                // conversion. I didn't plan this ahead, but that's neat
    _pageSize = ConfigProvider::get().page_size;
    _checkPages();
}

StackMemory::~StackMemory() {
    uint64_t size = STACK_TOP - _lowestAddr;
    uint64_t pageCount = size / _pageSize + (size % _pageSize != 0);
    for (size_t i = 0; i < pageCount; ++i) {
        _vmem.free(_lowestAddr + (_pageSize * i));
    }
}

// since redzone is 128 bytes in size, there's no need to run a page check immediately. it can wait
// until this cycle finishes to happen naturally. internal rsp doesn't have to be modified either,
// since it will be updated externally.
void StackMemory::push(uint64_t val) { write64(_rsp - 8, val); }

// with the assumptions above in place, pop can do just that, yep. deallocating pages or erasing
// data is lowkey pointless
uint64_t StackMemory::pop() { return read64(_rsp); }

void StackMemory::updateRsp(uint64_t rsp) {
    _rsp = rsp;
    _checkPages();
}

uint8_t StackMemory::read8(uint64_t vaddr) {
    _validateVaddr(vaddr, 1);
    return _vmem.read8(vaddr);
}
uint16_t StackMemory::read16(uint64_t vaddr) {
    _validateVaddr(vaddr, 2);
    return _vmem.read16(vaddr);
}
uint32_t StackMemory::read32(uint64_t vaddr) {
    _validateVaddr(vaddr, 4);
    return _vmem.read32(vaddr);
}
uint64_t StackMemory::read64(uint64_t vaddr) {
    _validateVaddr(vaddr, 8);
    return _vmem.read64(vaddr);
}

void StackMemory::write8(uint64_t vaddr, uint8_t value) {
    _validateVaddr(vaddr, 1);
    _vmem.write8(vaddr, value);
}
void StackMemory::write16(uint64_t vaddr, uint16_t value) {
    _validateVaddr(vaddr, 2);
    _vmem.write16(vaddr, value);
}
void StackMemory::write32(uint64_t vaddr, uint32_t value) {
    _validateVaddr(vaddr, 4);
    _vmem.write32(vaddr, value);
}
void StackMemory::write64(uint64_t vaddr, uint64_t value) {
    _validateVaddr(vaddr, 8);
    _vmem.write64(vaddr, value);
}
