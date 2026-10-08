#include "vm.hpp"

#include <iostream>

VirtualMachine::VirtualMachine(const VMConfig& config) : _state({}, {}, config) {
    _state.memory.init(&_state);
    _state.memory.mapRegion({0, 256, MEM_PERM_RWX, "r1"});
    _state.memory.mapRegion({256, 256, MEM_PERM_RX, "r2"});
    _state.memory.write8(199, 0);
    _state.memory.write32(200, 123456789);
    std::cout << _state.memory.read32(199) << std::endl;
    // _state.memory.write16(280, 123);
    std::cout << _state.memory.read16(280) << std::endl;
}
