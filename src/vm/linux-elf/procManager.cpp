#include "procManager.hpp"

#include <iostream>

#include "../../common/errors.hpp"
#include "../cfgProv.hpp"
#include "../x64/vmState.hpp"

ProcessManager::ProcessManager() {}

void assert(bool cond) {
    if (!cond)
        throw RuntimeError("test failed");
}

void ProcessManager::init(VMState* vmState) { _vmState = vmState; }

void ProcessManager::createProcess(int32_t pid, const std::string& elf, int32_t ppid) {
    _processes.emplace(std::piecewise_construct, std::make_tuple(pid),
                       std::make_tuple(_vmState, pid, ppid, elf));
}

// threads and processes will probably be alerting the scheduler about pausing/resuming, so we can
// just keep track whether there are ready ones or not
int ProcessManager::run() {
    if (_processes.size() == 0) {
        throw RuntimeError("ProcessManager: no processes to run");
    }
    uint64_t n = 999;
    auto it = _processes.begin();
    while (true) {
        if (it->second.state() == PROC_STATE_READY) {
            auto ctx = it->second.ctx();
            if (ctx.has_value()) {
                std::cout << "running process " << it->second.pid() << std::endl;
                _vmState->cpu.run(ctx.value(), n);
            }
        }
        if (++it == _processes.end()) {
            it = _processes.begin();
        }
    }

    // Process& root = _processes[0];
    // return root.exitCode();
}
