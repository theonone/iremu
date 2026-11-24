#include "heap.hpp"

#include "../../common/errors.hpp"
#include "../cfgProv.hpp"

HeapMemory::HeapMemory(VirtualMemory* vmem) : _vmem(vmem) {
    auto& cfg = ConfigProvider::get();
    _maxSize = cfg.max_heap;
    _pageSize = cfg.page_size;
}

HeapMemory::~HeapMemory() {
    uint64_t pages =
        (_mappedEnd - _base) / _pageSize;  // since both are page-aligned, simple division is enough
    for (size_t i = 0; i < pages; ++i) {
        _vmem->free(_base + (_pageSize * i));
    }
}

void HeapMemory::setBase(uint64_t base) {
    _base = base;
    _brk = base;
    _mappedEnd = base;
    if (base % _pageSize != 0) {
        throw MemoryError("Heap base isn't page-aligned");
    }
}

int HeapMemory::brk(uint64_t vaddr) {
    if (vaddr > _brk) {
        return (sbrk(vaddr - _brk) != UINT64_MAX) ? 0 : -1;
    } else if (_brk > vaddr) {
        return (sbrk(-int64_t(_brk - vaddr)) != UINT64_MAX) ? 0 : -1;
    }
    return 0;
}

uint64_t HeapMemory::sbrk(int64_t bytes) {
    uint64_t origBrk = _brk;

    if (bytes > 0) {
        if (uint64_t(bytes) > _mappedEnd - _brk) {
            uint64_t heapSize = uint64_t(_brk - _base) + bytes;
            uint64_t newPages = heapSize / _pageSize + (heapSize % _pageSize != 0);
            if (newPages * _pageSize > _maxSize) {
                return UINT64_MAX;
            }
            uint64_t size = bytes - (_mappedEnd - _brk);

            try {
                _vmem->allocate(size, _mappedEnd, MEM_PERM_RW);
            } catch (const MemoryError& _) {
                return UINT64_MAX;
            }
            uint64_t pages = size / _pageSize + (size % _pageSize != 0);
            _mappedEnd += pages * _pageSize;
        }
        _brk += bytes;
    } else if (bytes < 0) {
        uint64_t pos = -bytes;
        if (pos > (_brk - _base)) {
            return UINT64_MAX;
        }
        _brk -= pos;
        uint64_t size = _mappedEnd - _brk;
        uint64_t pages = size / _pageSize;
        for (size_t i = 0; i < pages; ++i) {
            _vmem->free(_mappedEnd - (_pageSize * (i + 1)));
        }
        _mappedEnd -= pages * _pageSize;
    }

    return origBrk;
}

uint64_t HeapMemory::mappedEnd() const { return _mappedEnd; }

uint64_t HeapMemory::base() const { return _base; }

inline void HeapMemory::_validateVaddr(uint64_t vaddr, uint64_t size) {
    if ((vaddr < _base) || (vaddr > _mappedEnd - 1 - size)) {
        throw MemoryError("Bad heap access");
    }
}

uint8_t HeapMemory::read8(uint64_t vaddr) {
    _validateVaddr(vaddr, 1);
    return _vmem->read8(vaddr);
}
uint16_t HeapMemory::read16(uint64_t vaddr) {
    _validateVaddr(vaddr, 2);
    return _vmem->read16(vaddr);
}
uint32_t HeapMemory::read32(uint64_t vaddr) {
    _validateVaddr(vaddr, 4);
    return _vmem->read32(vaddr);
}
uint64_t HeapMemory::read64(uint64_t vaddr) {
    _validateVaddr(vaddr, 8);
    return _vmem->read64(vaddr);
}

void HeapMemory::write8(uint64_t vaddr, uint8_t value) {
    _validateVaddr(vaddr, 1);
    _vmem->write8(vaddr, value);
}
void HeapMemory::write16(uint64_t vaddr, uint16_t value) {
    _validateVaddr(vaddr, 2);
    _vmem->write16(vaddr, value);
}
void HeapMemory::write32(uint64_t vaddr, uint32_t value) {
    _validateVaddr(vaddr, 4);
    _vmem->write32(vaddr, value);
}
void HeapMemory::write64(uint64_t vaddr, uint64_t value) {
    _validateVaddr(vaddr, 8);
    _vmem->write64(vaddr, value);
}