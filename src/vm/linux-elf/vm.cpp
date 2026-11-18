#include "vm.hpp"

#include <iostream>

#include "../x64/vmem.hpp"
#include "loader.hpp"

VirtualMachine::VirtualMachine(const VMConfig& config) : _state({}, {}, config) {
    _state.ram.init(&_state);

    // testing
    VirtualMemory vmem1;
    vmem1.init(&_state);
    uint64_t pageAddr = 5000;
    int64_t core = vmem1.allocate(2000, pageAddr, MEM_PERM_RWX);
    vmem1.write64(pageAddr, 256);
    std::cout << (vmem1.read64(pageAddr)) << std::endl;
    std::cout << (vmem1.read16(pageAddr)) << std::endl;
    std::cout << (vmem1.read8(pageAddr)) << std::endl;
    uint64_t largeAddr = 0xFFFFFFFFFF;
    vmem1.allocate(8000, largeAddr, MEM_PERM_RWX);
    ByteBuffer buff("Test 1234");
    vmem1.write(largeAddr, buff);
    std::cout << vmem1.read(largeAddr, buff.size()).toString() << std::endl;

    VirtualMemory vmem2;
    vmem2.init(&_state);
    uint64_t addr = 8190;
    vmem2.allocate(1, MEM_PERM_RWX);
    vmem2.allocate(1, MEM_PERM_RW);
    vmem2.write64(addr, UINT64_MAX);                 // cross-page write
    std::cout << (vmem2.read64(addr)) << std::endl;  // cross-page read

    uint64_t lastAddr = UINT64_MAX - 8192;
    vmem2.allocate(2048, lastAddr, MEM_PERM_RW);
    vmem2.write16(lastAddr, 12345);
    std::cout << (vmem2.read16(lastAddr)) << std::endl;

    for (const auto& p : vmem1.pageMap()) {
        std::cout << p.first << " - " << p.second.pages << std::endl;
    }

    for (const auto& p : vmem2.pageMap()) {
        std::cout << p.first << " - " << p.second.pages << std::endl;
    }
    // 18446744073709551615
    // 18446744073709547520
    // 18446744073709543424
    //

    // ExecutableLoader loader;
    // loader.init(&_state);
    // loader.load();

    std::cout << "done" << std::endl;
}
