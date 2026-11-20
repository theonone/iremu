#include "procManager.hpp"

#include "../../common/errors.hpp"

ProcessManager::ProcessManager() {}

void ProcessManager::init(VMState* state) { _state = state; }

void ProcessManager::run() {
    if (_processes.size() == 0) {
        throw RuntimeError("ProcessManager: no processes to run");
    }
    auto it = _processes.begin();
    while (true) {
    }
}
