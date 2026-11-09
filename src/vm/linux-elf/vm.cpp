#include "vm.hpp"

#include <iostream>

#include "loader.hpp"

VirtualMachine::VirtualMachine(const VMConfig& config) : _state({}, {}, config) {
    _state.memory.init(&_state);
    ExecutableLoader loader;
    loader.init(&_state);
    loader.load();
}
