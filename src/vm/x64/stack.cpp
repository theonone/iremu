#include "stack.hpp"

#include "../cfgProv.hpp"

StackMemory::StackMemory(VirtualMemory& vmem) : _vmem(vmem) {
    _maxSize = ConfigProvider::get()
                   .max_stack;  // if -1 i.e. unlimited, the value will turn into UINT64_MAX after
                                // conversion. I didn't plan this ahead, but that's neat
}

void StackMemory::push(uint64_t val) {}
uint64_t StackMemory::pop() {}

void StackMemory::checkRbp(uint64_t rbp) {}

uint8_t StackMemory::read8(uint64_t vaddr) {}
uint16_t StackMemory::read16(uint64_t vaddr) {}
uint32_t StackMemory::read32(uint64_t vaddr) {}
uint64_t StackMemory::read64(uint64_t vaddr) {}

void StackMemory::write8(uint64_t vaddr, uint8_t value) {}
void StackMemory::write16(uint64_t vaddr, uint16_t value) {}
void StackMemory::write32(uint64_t vaddr, uint32_t value) {}
void StackMemory::write64(uint64_t vaddr, uint64_t value) {}
