#include "vm.hpp"

#include "../cfgProv.hpp"

VirtualMachine::VirtualMachine(const VMConfig& config) : _state({}, {}, config) {
    ConfigProvider::set(config);
    _state.ram.init();
}

int VirtualMachine::run() {
    _processes.emplace(std::piecewise_construct, std::forward_as_tuple(0),
                       std::forward_as_tuple(&_state, 0, -1));
    return 0;
}
