#include "stack.hpp"

#include "../../common/errors.hpp"
#include "../cfgProv.hpp"

void StackMemory::_checkPages() {
    if (_rsp - REDZONE < _lowestVaddr) {
        uint64_t size = (_lowestVaddr + REDZONE) - _rsp;
        uint64_t pages = size / _pageSize + (size % _pageSize != 0);
        _lowestVaddr -= (pages * _pageSize);

        if (_topVaddr - _lowestVaddr > _maxSize) {
            throw MemoryError("Stack overflow. The limit of " + std::to_string(_maxSize) +
                              " was exceeded.");
        }
        _vmem.allocate(size, _lowestVaddr, MEM_PERM_RW);
    }
}

inline void StackMemory::_validateVaddr(uint64_t vaddr, uint64_t size) {
    if ((vaddr < _rsp - REDZONE) || (vaddr > _topVaddr - size)) {
        throw MemoryError("Bad stack access");
    }
}

StackMemory::StackMemory(VirtualMemory& vmem, uint64_t topVaddr, uint64_t maxSize)
    : _vmem(vmem), _maxSize(maxSize), _topVaddr(topVaddr) {
    _pageSize = ConfigProvider::get().page_size;
    if (_maxSize > (uint64_t(8) * 1024 * 1024 * 1024 * 1024) || _maxSize < _pageSize) {
        throw MemoryError("max_stack must be between page_size and 8TB");
    }
    _rsp = _topVaddr;
    _lowestVaddr = _topVaddr;
    _checkPages();
}

StackMemory::~StackMemory() {
    uint64_t size = _topVaddr - _lowestVaddr;
    uint64_t pageCount = size / _pageSize + (size % _pageSize != 0);
    for (size_t i = 0; i < pageCount; ++i) {
        _vmem.free(_lowestVaddr + (_pageSize * i));
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

uint64_t StackMemory::rsp() const { return _rsp; }

uint64_t StackMemory::topVaddr() const { return _topVaddr; }

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
