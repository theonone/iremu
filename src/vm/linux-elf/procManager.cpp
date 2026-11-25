#include "procManager.hpp"

#include <iostream>
#include <unordered_map>

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

struct BudgetInfo {
    uint64_t budget = 0;
    // 0 means no syscall is to be processed, > 0 - a syscall of that price is to be
    // executed once the budget is high enough
    uint64_t pendingSyscallPrice = 0;
};

// threads and processes will probably be alerting the scheduler about pausing/resuming, so we can
// just keep track whether there are ready ones or not
int ProcessManager::run() {
    if (_processes.size() == 0) {
        throw RuntimeError("ProcessManager: no processes to run");
    }
    std::unordered_map<uint64_t, BudgetInfo> budgets;

    const uint64_t instrBudget = 999;
    auto it = _processes.begin();
    while (true) {
        auto& proc = it->second;
        if (proc.state() == PROC_STATE_READY) {
            auto ctx = proc.ctx();
            if (ctx.has_value()) {
                uint64_t entryId = (uint64_t(ctx->pid) << 32) | (ctx->tid);
                auto& binf = budgets[entryId];
                binf.budget += instrBudget;

                if (binf.pendingSyscallPrice) {
                    if (binf.pendingSyscallPrice <= binf.budget) {
                        _kernel.syscall(ctx.value());
                        binf.budget -= binf.pendingSyscallPrice;
                        binf.pendingSyscallPrice = 0;
                    } else {
                        if (++it == _processes.end()) {
                            it = _processes.begin();
                        }
                        continue;
                    }
                }

                CPUInterrupt interrupt = _vmState->cpu.run(ctx.value(), binf.budget);
                binf.budget = interrupt.remaining;

                if (interrupt.reason == CPU_INT_SYSCALL) {
                    auto price = _kernel.syscallPrice(ctx.value());
                    if (price <= binf.budget) {
                        _kernel.syscall(ctx.value());
                        binf.budget -= price;
                    } else {
                        binf.pendingSyscallPrice = price;
                    }
                }
            }
        }
        if (++it == _processes.end()) {
            it = _processes.begin();
        }
    }

    // Process& root = _processes[0];
    // return root.exitCode();
}
