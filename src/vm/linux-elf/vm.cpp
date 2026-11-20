#include "vm.hpp"

#include "../cfgProv.hpp"

VirtualMachine::VirtualMachine(const VMConfig& config) : _vmState({}, {}) {
    ConfigProvider::set(config);
    _vmState.ram.init();
    _procManager.init(&_vmState);
}

int VirtualMachine::run() {
    _procManager.createProcess(0, ConfigProvider::get().target, -1);
    _procManager.createProcess(1, "./testing/benchmark2", 0);

    return _procManager.run();
}
