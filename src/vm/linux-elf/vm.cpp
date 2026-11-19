#include "vm.hpp"

#include "loader.hpp"

VirtualMachine::VirtualMachine(const VMConfig& config) : _state({}, {}, config) {
    _state.ram.init(&_state);
}

int VirtualMachine::run() {
    ExecutableLoader loader;
    loader.init(&_state);
    loader.load();
    _processes.emplace(0, &_state, 0, 0);
    return 0;
}
