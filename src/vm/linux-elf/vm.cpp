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
    int64_t core = vmem1.allocate(pageAddr, MEM_PERM_RWX, 2000);
    vmem1.write64(pageAddr, 256);
    std::cout << std::to_string(vmem1.read64(pageAddr)) << std::endl;
    std::cout << std::to_string(vmem1.read16(pageAddr)) << std::endl;
    std::cout << std::to_string(vmem1.read8(pageAddr)) << std::endl;
    uint64_t largeAddr = 0xFFFFFFFFFF;
    vmem1.allocate(8000, MEM_PERM_RWX, largeAddr);
    ByteBuffer buff("Test 1234");
    vmem1.write(largeAddr, buff);
    std::cout << vmem1.read(largeAddr, buff.size()).toString() << std::endl;

    VirtualMemory vmem2;
    vmem2.init(&_state);
    uint64_t addr = 8190;
    vmem2.allocate(1, MEM_PERM_RWX);
    vmem2.allocate(1, MEM_PERM_W);
    vmem2.write64(addr, UINT64_MAX);                               // cross-page write
    std::cout << std::to_string(vmem2.read64(addr)) << std::endl;  // cross-page read

    //

    // ExecutableLoader loader;
    // loader.init(&_state);
    // loader.load();

    std::cout << "done" << std::endl;
}
